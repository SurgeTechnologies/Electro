//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroEnvironmentMap.hpp"
#include "Core/ElectroVault.hpp"
#include "ElectroVertexBuffer.hpp"
#include "ElectroIndexBuffer.hpp"
#include "ElectroRenderCommand.hpp"
#include "ElectroFramebuffer.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Platform/DX11/DX11Internal.hpp"

namespace Electro
{
    //TODO: Abstract this out
    EnvironmentMap::EnvironmentMap(const String& hdrMapPath)
    {
        auto texture = Texture2D::Create(hdrMapPath);
        float vertices[] =
        {
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f
        };

        Uint indices[] =
        {
            0, 1, 3, 3, 1, 2,
            1, 5, 2, 2, 5, 6,
            5, 4, 6, 6, 4, 7,
            4, 0, 7, 7, 0, 3,
            3, 2, 7, 7, 2, 6,
            4, 5, 0, 0, 5, 1
        };

        ConstantBufferDesc cbdesc;
        cbdesc.BindSlot = 0;
        cbdesc.InitialData = nullptr;
        cbdesc.Name = "Camera";
        cbdesc.Shader = Vault::Get<Shader>("EquirectangularToCubemap.hlsl");
        cbdesc.ShaderDomain = ShaderDomain::VERTEX;
        cbdesc.Size = sizeof(glm::mat4);
        cbdesc.Usage = DataUsage::DYNAMIC;
        Ref<ConstantBuffer> cbuffer = ConstantBuffer::Create(cbdesc);

        //Pipeline for capturing the cube
        VertexBufferLayout templayout = { { ShaderDataType::Float3, "POSITION" } };
        Ref<VertexBuffer> tempvertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices), templayout);
        Ref<IndexBuffer> tempindexBuffer= IndexBuffer::Create(indices, static_cast<Uint>(std::size(indices)));
        PipelineSpecification tempPipelinespec;
        tempPipelinespec.VertexBuffer = tempvertexBuffer;
        tempPipelinespec.IndexBuffer = tempindexBuffer;
        tempPipelinespec.Shader = Vault::Get<Shader>("EquirectangularToCubemap.hlsl");
        auto tempPipeline = Pipeline::Create(tempPipelinespec);

        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] =
        {
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        };

        auto width = 512;
        auto height = 512;

        //Create the TextureCube
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 6;
        textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
        ID3D11Texture2D* tex = nullptr;
        DX_CALL(DX11Internal::GetDevice()->CreateTexture2D(&textureDesc, nullptr, &tex));

        //Shader Resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        DX_CALL(DX11Internal::GetDevice()->CreateShaderResourceView(tex, &srvDesc, &mSRV));

        //Create the Render target views
        Vector<ID3D11RenderTargetView*> rtvs;

        for (Uint i = 0; i < 6; i++)
        {
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = textureDesc.Format;
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            renderTargetViewDesc.Texture2DArray.MipSlice = 0;
            renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
            renderTargetViewDesc.Texture2DArray.ArraySize = 1;
            ID3D11RenderTargetView* view = nullptr;
            DX11Internal::GetDevice()->CreateRenderTargetView(tex, &renderTargetViewDesc, &view);
            rtvs.push_back(view);
        }

        auto deviceContext = DX11Internal::GetDeviceContext();

        texture->Bind(0);
        D3D11_VIEWPORT mViewport = {};
        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;
        mViewport.Width = static_cast<float>(512);
        mViewport.Height = static_cast<float>(512);
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;
        deviceContext->RSSetViewports(1, &mViewport);

        for (Uint i = 0; i < 6; i++)
        {
            float col[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
            deviceContext->ClearRenderTargetView(rtvs[i], col);
            deviceContext->OMSetRenderTargets(1, &rtvs[i], nullptr);
            tempPipeline->Bind();
            tempPipeline->BindSpecificationObjects();
            cbuffer->SetData(&captureViews[i]);
            cbuffer->Bind();
            RenderCommand::DrawIndexed(tempPipeline, 36);
        }

        Vault::Get<Framebuffer>("EditorLayerFramebuffer")->Bind();

        Vault::Get<Shader>("Skybox.hlsl")->Bind();
        ConstantBufferDesc desc;
        desc.Shader = Vault::Get<Shader>("Skybox.hlsl");
        desc.Name = "SkyboxCBuffer";
        desc.InitialData = nullptr;
        desc.Size = sizeof(glm::mat4);
        desc.BindSlot = 0;
        desc.ShaderDomain = ShaderDomain::VERTEX;
        desc.Usage = DataUsage::DYNAMIC;
        mSkyboxCBuffer = ConstantBuffer::Create(desc);

        //Pipeline for the skybox
        VertexBufferLayout layout = { { ShaderDataType::Float3, "SKYBOX_POS" } };
        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices), layout);
        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, static_cast<Uint>(std::size(indices)));
        PipelineSpecification spec;
        spec.VertexBuffer = vertexBuffer;
        spec.IndexBuffer = indexBuffer;
        spec.Shader = Vault::Get<Shader>("Skybox.hlsl");
        mPipeline = Pipeline::Create(spec);
        tex->Release();
    }

    void EnvironmentMap::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix)
    {
        RenderCommand::SetDepthTest(DepthTestFunc::LEqual);

        auto deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* sampler = DX11Internal::GetSkyboxSampler();

        mPipeline->Bind();
        mPipeline->BindSpecificationObjects();

        deviceContext->PSSetSamplers(1, 1, &sampler);
        deviceContext->PSSetShaderResources(0, 1, &mSRV);
        mSkyboxCBuffer->SetData((void*)&(projectionMatrix * glm::mat4(glm::mat3(viewMatrix))));
        mSkyboxCBuffer->Bind();
        Vault::Get<Shader>("Skybox.hlsl")->Bind();
        RenderCommand::DrawIndexed(mPipeline, 36);

        RenderCommand::SetDepthTest(DepthTestFunc::Less);
    }
}
