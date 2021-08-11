//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Renderer.hpp"
#include "Renderer2D.hpp"
#include "RenderCommand.hpp"
#include "EditorLayer.hpp"
#include "Math/Math.hpp"
#include "RenderPass/ShadowPass.hpp"
#include "RenderPass/DebugPass.hpp"
#include "RenderPass/GeometryPass.hpp"

namespace Electro
{
    Scope<RendererData> Renderer::sData = CreateScope<RendererData>();
    void Renderer::Init()
    {
        // Create All Shaders
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/PBR.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/Skybox.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/EquirectangularToCubemap.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/IrradianceConvolution.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/PreFilterConvolution.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/Renderer2DLine.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/ShadowMap.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/SolidColor.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/Outline.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/Grid.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/GaussianBlur.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/ThresholdDownsampleShader.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/Shaders/HLSL/QuadComposite.hlsl"));

         /*      CBufferGuide       */
         /*Binding -  Name          */
         /*--------|----------------*/
         /*   0    | Camera         */
         /*   1    | Mesh           */
         /*   2    | Material       */
         /*   3    | Lights         */
         /*   4    | Roughness      */
         /*   5    | Skybox         */
         /*   6    | LightMat       */
         /*   7    | ShadowSettings */
         /*   8    | InverseCamera  */
         /*   9    | Color          */
         /*  10    | BlurParams     */
         /*  11    | BloomThreshold */
         /*  12    | CompositeParams*/
         /*-------------------------*/

        // Create All ConstantBuffers (TODO: Come up with a proper way of managing all Constant Buffers)
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::mat4), 0, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::mat4), 1, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(Ref<ConstantBuffer>(nullptr)); // Used via Material
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(LightCBuffer), 3, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::vec4), 4, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(Ref<ConstantBuffer>(nullptr)); // Used via Material
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::mat4) * (NUM_CASCADES + 1), 6, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::vec4), 7, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::mat4), 8, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::vec4), 9, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(BlurParams), 10, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::vec4), 11, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::vec4) * 2, 12, DataUsage::DYNAMIC));

        sData->SceneCBuffer = sData->AllConstantBuffers[0];
        sData->TransformCBuffer = sData->AllConstantBuffers[1];
        sData->LightConstantBuffer = sData->AllConstantBuffers[3];
        sData->LightSpaceMatrixCBuffer = sData->AllConstantBuffers[6];
        sData->InverseViewProjectionCBuffer = sData->AllConstantBuffers[8];
        sData->SolidColorCBuffer = sData->AllConstantBuffers[9];
        sData->CompositeParamsCBuffer = sData->AllConstantBuffers[12];

        sData->QuadCompositeShader = GetShader("QuadComposite");

        Uint width = 1280;
        Uint height = 720;

        {
            RenderbufferSpecification fbSpec;
            fbSpec.Attachments = { RenderBufferTextureFormat::RGBA32F, RenderBufferTextureFormat::DEPTH };
            fbSpec.Width = width;
            fbSpec.Height = height;
            fbSpec.SwapChainTarget = false;
            fbSpec.DebugName = "Geometry Texture";
            sData->GeometryBuffer = Renderbuffer::Create(fbSpec);

            fbSpec.DebugName = "Final Texture";
            sData->FinalSceneBuffer = Renderbuffer::Create(fbSpec);
        }

        sData->RenderPassManager.Init(sData.get());
        sData->RenderPassManager.AddRenderPass(new ShadowPass);
        sData->RenderPassManager.AddRenderPass(new GeometryPass);
        sData->RenderPassManager.AddRenderPass(new DebugPass);

        sData->PostProcessPipeline.Init(sData->GeometryBuffer);
    }

    void Renderer::Shutdown()
    {
        sData->RenderPassManager.Shutdown();
        sData->PostProcessPipeline.Shutdown();
        sData.release();
    }

    void Renderer::BeginScene(EditorCamera& camera)
    {
        sData->ViewProjectionMatrix = camera.GetViewProjection();
        sData->ProjectionMatrix = camera.GetProjection();
        sData->ViewMatrix = camera.GetViewMatrix();
        sData->CameraPosition = camera.GetPosition();
    }

    void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        sData->ViewProjectionMatrix = camera.GetProjection() * glm::inverse(transform);
        sData->ProjectionMatrix = camera.GetProjection();
        sData->ViewMatrix = glm::inverse(transform);

        glm::vec3 translation, rotation, scale;
        Math::DecomposeTransform(transform, translation, rotation, scale);
        sData->CameraPosition = translation;
    }

    void Renderer::SubmitMesh(const MeshComponent& mesh, const glm::mat4& transform)
    {
        sData->MeshDrawList.emplace_back(mesh, transform);
    }

    void Renderer::SubmitOutlineMesh(const MeshComponent& mesh, const glm::mat4& transform)
    {
        sData->OutlineDrawList.emplace_back(mesh, transform);
    }

    void Renderer::SubmitColliderMesh(const BoxColliderComponent& component, const glm::mat4& transform, bool show)
    {
        sData->ColliderDrawList.emplace_back(component.DebugMesh, transform, show);
    }

    void Renderer::SubmitColliderMesh(const SphereColliderComponent& component, const glm::mat4& transform, bool show)
    {
        sData->ColliderDrawList.emplace_back(component.DebugMesh, transform, show);
    }

    void Renderer::SubmitColliderMesh(const MeshColliderComponent& component, const glm::mat4& transform, bool show)
    {
        for (const Ref<Mesh>& debugMesh : component.ProcessedMeshes)
            sData->ColliderDrawList.emplace_back(debugMesh, transform, show);
    }

    void Renderer::SubmitColliderMesh(const CapsuleColliderComponent& component, const glm::mat4& transform, bool show)
    {
        sData->ColliderDrawList.emplace_back(component.DebugMesh, transform, show);
    }

    void Renderer::SubmitPointLight(const PointLight& pointLight)
    {
        sData->AllPointLights.emplace_back(pointLight);
    }

    void Renderer::SubmitDirectionalLight(const DirectionalLight& directionalLight)
    {
        sData->AllDirectionalLights.emplace_back(directionalLight);
    }

    void Renderer::PostProcessing()
    {
        sData->PostProcessPipeline.ProcessAll();
    }

    void Renderer::CompositePass()
    {
        sData->FinalSceneBuffer->Clear({ 0.0f, 0.0f, 0.0f, 0.0f });
        const Ref<Renderbuffer>& bloomResult = sData->PostProcessPipeline.GetEffectByKey<Bloom>(BLOOM_METHOD_KEY)->GetOutputRenderBuffer();

        // We are now rendering to the Final Scene RendererBuffer
        if (sData->RenderToSwapChain)
        {
            const RenderbufferSpecification& rbSpec = RenderCommand::GetBackBuffer()->GetSpecification();
            sData->CompositeParams.InverseScreenSize = glm::vec2(1.0f / rbSpec.Width, 1.0f / rbSpec.Height);
            RenderCommand::BindBackbuffer();
        }
        else
        {
            const RenderbufferSpecification& rbSpec = sData->FinalSceneBuffer->GetSpecification();
            sData->CompositeParams.InverseScreenSize = glm::vec2(1.0f / rbSpec.Width, 1.0f / rbSpec.Height);
            sData->FinalSceneBuffer->Bind();
        }

        sData->QuadCompositeShader->Bind();

        // Bind the Blurred texture and the Geometry Buffer
        sData->GeometryBuffer->BindColorBuffer(0, 0, ShaderDomain::PIXEL);
        bloomResult->BindColorBuffer(0, 1, ShaderDomain::PIXEL);

        sData->CompositeParamsCBuffer->SetDynamicData(&sData->CompositeParams);
        sData->CompositeParamsCBuffer->PSBind();

        RenderFullscreenQuad();

        // Unbind the Blurred texture and the Geometry Buffer
        bloomResult->UnbindBuffer(1, ShaderDomain::PIXEL);
        sData->GeometryBuffer->UnbindBuffer(0, ShaderDomain::PIXEL);

        if (sData->RenderToSwapChain)
            RenderCommand::GetBackBuffer()->Unbind();
        else
            sData->FinalSceneBuffer->Unbind();
    }

    void Renderer::EndScene()
    {
        sData->RenderPassManager.GetRenderPass<ShadowPass>()->Update();
        sData->RenderPassManager.GetRenderPass<GeometryPass>()->Update();
        PostProcessing();

        // Render the skybox
        sData->GeometryBuffer->Bind();
        if (sData->EnvironmentMap && sData->EnvironmentMapActivated)
            sData->EnvironmentMap->Render(sData->ProjectionMatrix, sData->ViewMatrix);
        sData->GeometryBuffer->Unbind();

        if (!sData->SceneContext->IsRuntimeScene())
            sData->RenderPassManager.GetRenderPass<DebugPass>()->Update();

        CompositePass();
        ClearDrawList();
    }

    void Renderer::ClearDrawList()
    {
        sData->MeshDrawList.clear();
        sData->OutlineDrawList.clear();
        sData->ColliderDrawList.clear();
    }

    void Renderer::RenderFullscreenQuad()
    {
        RenderCommand::DisableDepth();
        RenderCommand::Draw(3);
        RenderCommand::EnableDepth();
    }

    const Ref<Shader> Renderer::GetShader(const String& nameWithoutExtension)
    {
        for (const Ref<Shader>& shader : sData->AllShaders)
        {
            if (FileSystem::GetNameWithoutExtension(shader->GetPath()) == nameWithoutExtension)
                return shader;
        }
        return Ref<Shader>(nullptr);
    }

    void Renderer::OnWindowResize(Uint width, Uint height)
    {
        RenderCommand::ResizeBackbuffer(0, 0, width, height);
    }

    void Renderer::RenderToSwapchain()
    {
        sData->RenderToSwapChain = true;
    }
}
