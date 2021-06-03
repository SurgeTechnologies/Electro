//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Cascades.hpp"
#include "RendererDebug.hpp"
#include "Factory.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Electro
{
    void Cascades::Init()
    {
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::R32_TYPELESS };
        fbSpec.Width = SHADOW_MAP_RESOLUTION;
        fbSpec.Height = SHADOW_MAP_RESOLUTION;
        fbSpec.SwapChainTarget = false;

        for(Ref<Framebuffer>& shadowMap : mShadowMaps)
            shadowMap = Factory::CreateFramebuffer(fbSpec);
    }

    void Cascades::CalculateViewProjection(glm::mat4& view, const glm::mat4& projection, const glm::vec3& normalizedDirection)
    {
        glm::mat4 viewProjection = projection * view;
        glm::mat4 inverseViewProjection = glm::inverse(viewProjection);

        // TODO: Automate this
        const float nearClip = 0.1f;
        const float farClip = 1000.0f;
        const float clipRange = farClip - nearClip;

        // Calculate the optimal cascade distances
        const float minZ = nearClip;
        const float maxZ = nearClip + clipRange;
        const float range = maxZ - minZ;
        const float ratio = maxZ / minZ;
        for (Uint i = 0; i < NUM_CASCADES; i++)
        {
            const float p = (i + 1) / static_cast<float>(NUM_CASCADES);
            const float log = minZ * glm::pow(ratio, p);
            const float uniform = minZ + range * p;
            const float d = CASCADE_SPLIT_LAMBDA * (log - uniform) + uniform;
            mCascadeSplits[i] = (d - nearClip) / clipRange;
        }

        float lastSplitDist = 0.0f;
        // Calculate Orthographic Projection matrix for each cascade
        for (Uint cascade = 0; cascade < NUM_CASCADES; cascade++)
        {
            float splitDist = mCascadeSplits[cascade];
            glm::vec4 frustumCorners[NUM_FRUSTUM_CORNERS] =
            {
                //Near face
                {  1.0f,  1.0f, -1.0f, 1.0f },
                { -1.0f,  1.0f, -1.0f, 1.0f },
                {  1.0f, -1.0f, -1.0f, 1.0f },
                { -1.0f, -1.0f, -1.0f, 1.0f },

                //Far face
                {  1.0f,  1.0f, 1.0f, 1.0f },
                { -1.0f,  1.0f, 1.0f, 1.0f },
                {  1.0f, -1.0f, 1.0f, 1.0f },
                { -1.0f, -1.0f, 1.0f, 1.0f },
            };

            // Project frustum corners into world space from clip space
            for (Uint i = 0; i < NUM_FRUSTUM_CORNERS; i++)
            {
                glm::vec4 invCorner = inverseViewProjection * frustumCorners[i];
                frustumCorners[i] = invCorner / invCorner.w;
            }
            for (Uint i = 0; i < 4; i++)
            {
                glm::vec4 dist = frustumCorners[i + 4] - frustumCorners[i];
                frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
                frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
            }

            // Get frustum center
            glm::vec3 frustumCenter = glm::vec3(0.0f);
            for (Uint i = 0; i < NUM_FRUSTUM_CORNERS; i++)
                frustumCenter += glm::vec3(frustumCorners[i]);
            frustumCenter /= 8.0f;

            // Get the minimum and maximum extents
            float radius = 0.0f;
            for (Uint i = 0; i < NUM_FRUSTUM_CORNERS; i++)
            {
                float distance = glm::length(glm::vec3(frustumCorners[i]) - frustumCenter);
                radius = glm::max(radius, distance);
            }
            radius = std::ceil(radius * 16.0f) / 16.0f;
            glm::vec3 maxExtents = glm::vec3(radius);
            glm::vec3 minExtents = -maxExtents;

            // Calculate the view and projection matrix
            glm::vec3 lightDir = -normalizedDirection;
            glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 lightProjectionMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, -15.0f, maxExtents.z - minExtents.z + 15.0f);

            // Offset to texel space to avoid shimmering ->(https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
            glm::mat4 shadowMatrix = lightProjectionMatrix * lightViewMatrix;
            const float ShadowMapResolution = SHADOW_MAP_RESOLUTION;
            glm::vec4 shadowOrigin = (shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) * ShadowMapResolution / 2.0f;
            glm::vec4 roundedOrigin = glm::round(shadowOrigin);
            glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
            roundOffset = roundOffset * 2.0f / ShadowMapResolution;
            roundOffset.z = 0.0f;
            roundOffset.w = 0.0f;
            lightProjectionMatrix[3] += roundOffset;

            // Store SplitDistance and ViewProjection-Matrix
            mCascadeSplitDepths[cascade] = (nearClip + splitDist * clipRange) * 1.0f;
            mViewProjections[cascade] = lightProjectionMatrix * lightViewMatrix;
            lastSplitDist = mCascadeSplits[cascade];

            // -----------------------Debug only-----------------------
             RendererDebug::BeginScene(viewProjection);
             RendererDebug::SubmitCameraFrustum(frustumCorners, glm::mat4(1.0f), GetColor(cascade));   // Draws the divided camera frustums
             RendererDebug::SubmitLine(glm::vec3(0.0f, 0.0f, 0.0f), frustumCenter, GetColor(cascade)); // Draws the center of the frustum (A line pointing from origin to the center)
             RendererDebug::EndScene();
        }
    }

    void Cascades::Bind(Uint slot) const
    {
        for(Uint i = 0; i < NUM_CASCADES; i++)
            mShadowMaps[i]->BindDepthBuffer(slot + i);
    }

    void Cascades::Unbind(Uint slot) const
    {
        for (Uint i = 0; i < NUM_CASCADES; i++)
            mShadowMaps[i]->UnbindDepthBuffer(slot + i);
    }

    glm::vec4 Cascades::GetColor(Uint cascade)
    {
        if (cascade == 0)
            return { 1.0, 0.0, 0.0f, 1.0f };
        if (cascade == 1)
            return { 0.0, 1.0, 0.0f, 1.0f };
        if (cascade == 2)
            return { 0.0, 0.0, 1.0f, 1.0f };
        if (cascade == 3)
            return { 1.0, 1.0, 0.0f, 1.0f };
        return {};
    }
}
