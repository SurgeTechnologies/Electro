//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Cascades.hpp"
#include "Factory.hpp"
#include "Math/BoundingBox.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "RendererDebug.hpp"
#include "Platform/DX11/DX11Internal.hpp"

#define NUM_FRUSTUM_CORNERS 8
#define M_PI       3.14159265358979323846
#define ToRadian(x) (float)(((x) * M_PI / 180.0f))

namespace Electro
{
    void Cascades::Init()
    {
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::R32_TYPELESS };
        fbSpec.Width = 4096;
        fbSpec.Height = 4096;
        fbSpec.SwapChainTarget = false;

        for(Ref<Framebuffer>& shadowMap : mShadowMaps)
            shadowMap = Factory::CreateFramebuffer(fbSpec);
    }

    inline glm::vec4 GetColor(Uint cascade)
    {
        if(cascade == 0)
            return { 1.0, 0.0, 0.0f, 1.0f };
        if(cascade == 1)
            return { 0.0, 1.0, 0.0f, 1.0f };
        if(cascade == 2)
            return { 0.0, 0.0, 1.0f, 1.0f };
        if(cascade == 3)
            return { 1.0, 1.0, 0.0f, 1.0f };
        return {};
    }

    void Cascades::CalculateViewProjection(glm::mat4& view, const glm::mat4& projection, const glm::vec3& normalizedDirection)
    {
        glm::mat4 viewProjection = projection * view;

        // TODO: Automate this
        const float nearClip = 0.1f;
        const float farClip = 1000.0f;

        const float clipRange = farClip - nearClip;
        const float minZ = nearClip;
        const float maxZ = nearClip + clipRange;
        const float range = maxZ - minZ;
        const float ratio = maxZ / minZ;

        // Calculate the optimal cascade distances
        for (Uint i = 0; i < NUM_CASCADES; i++)
        {
            const float p = (i + 1) / static_cast<float>(NUM_CASCADES);
            const float log = minZ * std::pow(ratio, p);
            const float uniform = minZ + range * p;
            const float d = 0.91f * (log - uniform) + uniform;
            mCascadeSplits[i] = (d - nearClip) / clipRange;
        }

        // Calculate Orthographic Projection matrix for each cascade
        float lastSplitDist = 0.0f;
        for (Uint cascade = 0; cascade < NUM_CASCADES; cascade++)
        {
            float splitDist = mCascadeSplits[cascade];
            glm::vec4 frustumCorners[8] =
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

            // Project frustum corners into world space
            glm::mat4 invCam = glm::inverse(viewProjection);
            for (Uint i = 0; i < 8; i++)
            {
                glm::vec4 invCorner = invCam * frustumCorners[i];
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
            for (Uint i = 0; i < 8; i++)
                frustumCenter += glm::vec3(frustumCorners[i]);
            frustumCenter /= 8.0f;

            float radius = 0.0f;
            for (Uint i = 0; i < 8; i++)
            {
                float distance = glm::length(glm::vec3(frustumCorners[i]) - frustumCenter);
                radius = glm::max(radius, distance);
            }
            radius = std::ceil(radius * 16.0f) / 16.0f;

            glm::vec3 maxExtents = glm::vec3(radius);
            glm::vec3 minExtents = -maxExtents;

            glm::vec3 lightDir = -normalizedDirection;
            glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 lightProjectionMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f + mCascadeNearPlaneOffset, maxExtents.z - minExtents.z + mCascadeFarPlaneOffset);

            // Offset to texel space to avoid shimmering (https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
            glm::mat4 shadowMatrix = lightProjectionMatrix * lightViewMatrix;
            const float ShadowMapResolution = 4096.0f;
            glm::vec4 shadowOrigin = (shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) * ShadowMapResolution / 2.0f;
            glm::vec4 roundedOrigin = glm::round(shadowOrigin);
            glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
            roundOffset = roundOffset * 2.0f / ShadowMapResolution;
            roundOffset.z = 0.0f;
            roundOffset.w = 0.0f;
            lightProjectionMatrix[3] += roundOffset;

            // Debug only
            RendererDebug::BeginScene(viewProjection);
            // Draw the frustum
            RendererDebug::SubmitCameraFrustum(frustumCorners, glm::mat4(1.0f), GetColor(cascade));
            // Draw the center of the frustum (A line pointing from origin to the center)
            RendererDebug::SubmitLine(glm::vec3(0.0f, 0.0f, 0.0f), frustumCenter, GetColor(cascade));
            RendererDebug::EndScene();

            // Store split distance and matrix in cascade
            mCascadeSplitDepth[cascade] = (nearClip + splitDist * clipRange) * 1.0f;
            mViewProjections[cascade] = lightProjectionMatrix * lightViewMatrix;
            lastSplitDist = mCascadeSplits[cascade];
        }
    }

    //TODO: Abstract these out to DX11Framebuffer
    void Cascades::Bind(Uint slot) const
    {
        for(Uint i = 0; i < NUM_CASCADES; i++)
        {
            ID3D11ShaderResourceView* dsrv = static_cast<ID3D11ShaderResourceView*>(mShadowMaps[i]->GetDepthAttachmentID());
            DX11Internal::GetDeviceContext()->PSSetShaderResources(slot + i, 1, &dsrv);
        }
    }
    void Cascades::Unbind(Uint slot) const
    {
        ID3D11ShaderResourceView* null = nullptr;
        for(Uint i = 0; i < NUM_CASCADES; i++)
            DX11Internal::GetDeviceContext()->PSSetShaderResources(slot + i, 1, &null);
    }
}
