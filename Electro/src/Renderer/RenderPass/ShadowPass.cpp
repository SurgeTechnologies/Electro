//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ShadowPass.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Components.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/RenderCommand.hpp"

namespace Electro
{
    void ShadowPass::Init(RendererData* rendererData)
    {
        mRendererData = rendererData;
        mData.ShadowMapShader = Renderer::GetShader("ShadowMap");

        RenderbufferSpecification fbSpec;
        fbSpec.Attachments = { RenderBufferTextureFormat::SHADOW };
        fbSpec.Width = mData.ShadowMapResolution;
        fbSpec.Height = mData.ShadowMapResolution;
        fbSpec.SwapChainTarget = false;

        for (Ref<Renderbuffer>& shadowMap : mData.ShadowMaps)
            shadowMap = Renderbuffer::Create(fbSpec);

        mData.ShadowCBuffer = Renderer::GetConstantBuffer(7);
    }

    void ShadowPass::Update()
    {
        glm::vec3 direction;
        if (mRendererData->SceneContext)
        {
            auto view = mRendererData->SceneContext->GetRegistry().view<TransformComponent, DirectionalLightComponent>();
            for (const entt::entity& entity : view)
            {
                auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);
                direction = transform.GetTransform()[2]; // Z axis of rotation matrix
            }
        }

        // Calculate the ViewProjection matrices
        CalculateMatricesAndUpdateCBuffer(mRendererData->ViewMatrix, mRendererData->ProjectionMatrix, glm::normalize(direction));

        // Loop over all the shadow maps and bind and render the whole scene to each of them
        for (Uint j = 0; j < NUM_CASCADES; j++)
        {
            const Ref<Renderbuffer>& shadowMapBuffer = mData.ShadowMaps[j];
            shadowMapBuffer->Bind();
            mData.ShadowMapShader->Bind();
            shadowMapBuffer->Clear();

            // Set the LightSpaceMatrix
            mRendererData->SceneCBuffer->VSBind();
            mRendererData->SceneCBuffer->SetDynamicData((void*)(&mData.ViewProjections[j]));

            for (const DrawCommand& drawCmd : mRendererData->MeshDrawList)
            {
                if (drawCmd.GetMeshComponent().CastShadows)
                {
                    const Ref<Mesh>& mesh = drawCmd.GetMesh();
                    const Submesh* submeshes = mesh->GetSubmeshes().data();

                    const Ref<Pipeline>& pipeline = mesh->GetPipeline();
                    mesh->GetVertexBuffer()->Bind(pipeline->GetStride());
                    mesh->GetIndexBuffer()->Bind();
                    pipeline->Bind();

                    for (Uint i = 0; i < mesh->GetSubmeshes().size(); i++)
                    {
                        const Submesh& submesh = submeshes[i];
                        mRendererData->TransformCBuffer->VSBind();
                        mRendererData->TransformCBuffer->SetDynamicData(&(drawCmd.GetTransform() * submesh.Transform));
                        RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                    }
                    pipeline->Unbind();
                }
            }
        }
    }

    void ShadowPass::CalculateMatricesAndUpdateCBuffer(glm::mat4& view, const glm::mat4& projection, const glm::vec3& normalizedDirection)
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
            const float d = mData.CascadeSplitLambda * (log - uniform) + uniform;
            mData.CascadeSplits[i] = (d - nearClip) / clipRange;
        }

        float lastSplitDist = 0.0f;
        // Calculate Orthographic Projection matrix for each cascade
        for (Uint cascade = 0; cascade < NUM_CASCADES; cascade++)
        {
            float splitDist = mData.CascadeSplits[cascade];
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
            for (glm::vec4& frustumCorner : frustumCorners)
            {
                glm::vec4 invCorner = inverseViewProjection * frustumCorner;
                frustumCorner = invCorner / invCorner.w;
            }

            for (Uint i = 0; i < 4; i++)
            {
                glm::vec4 dist = frustumCorners[i + 4] - frustumCorners[i];
                frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
                frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
            }

            // Get frustum center
            glm::vec3 frustumCenter = glm::vec3(0.0f);
            for (glm::vec4& frustumCorner : frustumCorners)
                frustumCenter += glm::vec3(frustumCorner);
            frustumCenter /= 8.0f;

            // Get the minimum and maximum extents
            float radius = 0.0f;
            for (glm::vec4& frustumCorner : frustumCorners)
            {
                float distance = glm::length(glm::vec3(frustumCorner) - frustumCenter);
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
            glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            shadowOrigin = shadowMatrix * shadowOrigin;
            float storedW = shadowOrigin.w;
            shadowOrigin = shadowOrigin * static_cast<float>(mData.ShadowMapResolution) / 2.0f;
            glm::vec4 roundedOrigin = glm::round(shadowOrigin);
            glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
            roundOffset = roundOffset * 2.0f / static_cast<float>(mData.ShadowMapResolution);
            roundOffset.z = 0.0f;
            roundOffset.w = 0.0f;
            glm::mat4 shadowProj = lightProjectionMatrix;
            shadowProj[3] += roundOffset;
            lightProjectionMatrix = shadowProj;

            // Store SplitDistance and ViewProjection-Matrix
            mData.CascadeSplitDepths[cascade] = (nearClip + splitDist * clipRange) * 1.0f;
            mData.ViewProjections[cascade] = lightProjectionMatrix * lightViewMatrix;
            lastSplitDist = mData.CascadeSplits[cascade];
        }

        mData.ShadowCBuffer->SetDynamicData(&mData.CascadeSplitDepths);
        mData.ShadowCBuffer->PSBind();
    }


    void ShadowPass::Shutdown()
    {
    }
}

