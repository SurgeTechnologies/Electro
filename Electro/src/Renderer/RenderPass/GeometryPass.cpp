//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "GeometryPass.hpp"
#include "Renderer/Renderer.hpp"
#include "ShadowPass.hpp"
#include "Renderer/RenderCommand.hpp"

#define SHADOW_MAP_BINDING_SLOT 8

namespace Electro
{
    void GeometryPass::Init(RendererData* rendererData)
    {
        mRendererData = rendererData;
    }

    void GeometryPass::Update()
    {
        const Ref<Renderbuffer>& targetRenderbuffer = mRendererData->GeometryBuffer;

        targetRenderbuffer->Bind();
        targetRenderbuffer->Clear({ 0.05f, 0.05f, 0.05f, 1.0f });

        mRendererData->InverseViewProjectionCBuffer->VSBind();
        mRendererData->InverseViewProjectionCBuffer->SetDynamicData(&glm::inverse(mRendererData->ViewProjectionMatrix));

        mRendererData->SceneCBuffer->VSBind();
        mRendererData->SceneCBuffer->SetDynamicData(&mRendererData->ViewProjectionMatrix);

        ShadowPass* shadowPass = mRendererData->RenderPassManager.GetRenderPass<ShadowPass>();
        ShadowPass::InternalData* shadowData = mRendererData->RenderPassManager.GetRenderPassData<ShadowPass>();

        // Loop over the total number of cascades and set the light ViewProjection
        glm::mat4 lightMatData[NUM_CASCADES + 1];
        for (Uint i = 0; i < NUM_CASCADES; i++)
            lightMatData[i] = shadowData->ViewProjections[i];

        // Set the the view matrix at the last index
        lightMatData[NUM_CASCADES] = mRendererData->ViewMatrix;

        // Set the LightMatrix Data to the Vertex shader
        mRendererData->LightSpaceMatrixCBuffer->VSBind();
        mRendererData->LightSpaceMatrixCBuffer->SetDynamicData(lightMatData);

        // Update the Lights, set the LightCBufferData
        mRendererData->LightCBufferData.CameraPosition = mRendererData->CameraPosition;
        mRendererData->LightCBufferData.PointLightCount = static_cast<Uint>(mRendererData->AllPointLights.size());
        mRendererData->LightCBufferData.DirectionalLightCount = static_cast<Uint>(mRendererData->AllDirectionalLights.size());
        for (int i = 0; i < mRendererData->AllPointLights.size(); i++)
        {
            auto& light = mRendererData->AllPointLights[i];
            mRendererData->LightCBufferData.PointLights[i].Position = light.Position;
            mRendererData->LightCBufferData.PointLights[i].Intensity = light.Intensity;
            mRendererData->LightCBufferData.PointLights[i].Color = light.Color;
            mRendererData->LightCBufferData.PointLights[i].Radius = light.Radius;
        }
        for (int i = 0; i < mRendererData->AllDirectionalLights.size(); i++)
        {
            auto& light = mRendererData->AllDirectionalLights[i];
            mRendererData->LightCBufferData.DirectionalLights[i].Direction = light.Direction;
            mRendererData->LightCBufferData.DirectionalLights[i].Intensity = light.Intensity;
            mRendererData->LightCBufferData.DirectionalLights[i].Color = light.Color;
        }
        mRendererData->LightConstantBuffer->PSBind();
        mRendererData->LightConstantBuffer->SetDynamicData(&mRendererData->LightCBufferData);

        // Bind the shadow maps(which was captured from the ShadowPass()) as texture and draw all the objects in the scene
        //! NOTE: Here starting slot is SHADOW_MAP_BINDING_SLOT = 8, so the shadow maps gets bound as 8, 9, 10, ..., n
        shadowPass->BindAllShadowMaps(SHADOW_MAP_BINDING_SLOT);

        for (const DrawCommand& drawCmd : mRendererData->MeshDrawList)
        {
            const Ref<Mesh>& mesh = drawCmd.GetMesh();
            const Ref<Pipeline>& pipeline = mesh->GetPipeline();
            mesh->GetVertexBuffer()->Bind(pipeline->GetStride());
            mesh->GetIndexBuffer()->Bind();
            pipeline->Bind();

            const Vector<Ref<Material>>& materials = mesh->GetMaterials();
            const Submesh* submeshes = mesh->GetSubmeshes().data();

            // Render all the submeshes
            for (Uint i = 0; i < mesh->GetSubmeshes().size(); i++)
            {
                const Submesh& submesh = submeshes[i];
                materials[submesh.MaterialIndex]->Bind();

                mRendererData->TransformCBuffer->VSBind();
                mRendererData->TransformCBuffer->SetDynamicData(&(drawCmd.GetTransform() * submesh.Transform));
                RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                materials[submesh.MaterialIndex]->Unbind();
            }

            pipeline->Unbind();
        }

        shadowPass->UnbindAllShadowMaps(SHADOW_MAP_BINDING_SLOT);

        // Clear All the lights
        mRendererData->AllDirectionalLights.clear();
        mRendererData->AllPointLights.clear();

        targetRenderbuffer->Unbind();
    }

    void GeometryPass::Shutdown()
    {
    }
}