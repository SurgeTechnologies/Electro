//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Bloom.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/RenderCommand.hpp"

namespace Electro
{
    void Bloom::Init(const Ref<Renderbuffer>& target)
    {
        mTarget = target;
        RenderbufferSpecification targetSpec = target->GetSpecification();

        RenderbufferSpecification fbSpec;
        fbSpec.Attachments = { RenderBufferTextureFormat::RGBA32F };
        fbSpec.Width = targetSpec.Width / 2;
        fbSpec.Height = targetSpec.Height / 2;
        fbSpec.SwapChainTarget = false;
        fbSpec.Flags = RenderBufferFlags::COMPUTEWRITE;
        for (Uint i = 0; i < 2; i++)
            mBloomRenderTargets[i] = Renderbuffer::Create(fbSpec);

        mGaussianBlurShader = Renderer::GetShader("GaussianBlur");
        mThresholdDownsampleShader = Renderer::GetShader("ThresholdDownsampleShader");

        mBlurParamsCBuffer =  Renderer::GetConstantBuffer(10);
        mBloomThresholdCBuffer = Renderer::GetConstantBuffer(11);
    }

    void Bloom::Process()
    {
        if (!mBloomEnabled)
            return;

        //TODO Optimize, don't do every frame
        CalculateGaussianCoefficients();

        // Make sure that the bloom render targets get resized as too
        for (Uint i = 0; i < 2; i++)
        {
            const RenderbufferSpecification& fbSpec = mTarget->GetSpecification();
            mBloomRenderTargets[i]->EnsureSize(fbSpec.Width / 2, fbSpec.Height / 2);
        }

        // Extract the brightness + downsample
        {
            glm::vec4 thresholdParams = { mBloomThreshold, 0.0f, 0.0f, 0.0f };

            // Bind the downsample shader
            mThresholdDownsampleShader->Bind();

            mBloomThresholdCBuffer->SetDynamicData(&thresholdParams);
            mBloomThresholdCBuffer->CSBind();

            // Bind input and output texture
            mBloomRenderTargets[0]->CSBindUAV(0, 0);
            mTarget->BindColorBuffer(0, 0, ShaderDomain::COMPUTE);

            const RenderbufferSpecification& spec = mTarget->GetSpecification();
            RenderCommand::DispatchCompute(spec.Width / 16, spec.Height / 16, 1);

            // Unbind all the bound textures
            mTarget->UnbindBuffer(0, ShaderDomain::COMPUTE);
            mBloomRenderTargets[0]->CSUnbindUAV(0);
        }

        // Gaussian blur (in two passes)
        {
            // Bind the blur shader
            mGaussianBlurShader->Bind();

            auto& renderTargets = mBloomRenderTargets;
            Ref<Renderbuffer> csSRVs[2] = { renderTargets[0], renderTargets[1] };
            Ref<Renderbuffer> csUAVs[2] = { renderTargets[1], renderTargets[0] };

            for (Uint direction = 0; direction < 2; ++direction)
            {
                // Upload the Blur Params data to Shader
                mBlurParams.Direction = direction;
                mBlurParamsCBuffer->SetDynamicData(&mBlurParams);
                mBlurParamsCBuffer->CSBind();

                // Blurr The Texture
                csSRVs[direction]->BindColorBuffer(0, 0, ShaderDomain::COMPUTE);
                csUAVs[direction]->CSBindUAV(0, 0);

                const RenderbufferSpecification& spec = mTarget->GetSpecification();
                RenderCommand::DispatchCompute(spec.Width / 16, spec.Height / 16, 1);

                csSRVs[direction]->UnbindBuffer(0, ShaderDomain::COMPUTE);
                csUAVs[direction]->CSUnbindUAV(0);
            }
        }
    }

    void Bloom::CalculateGaussianCoefficients()
    {
        mBlurParams.Radius = GAUSSIAN_RADIUS;
        mBlurParams.Direction = 0;

        // Compute Gaussian kernel
        float& sigma = mBlurParams.GaussianSigma;
        float sigmaRcp = 1.0f / sigma;
        float twoSigmaSq = 2.0f * sigma * sigma;

        float sum = 0.0f;
        for (Uint i = 0; i <= GAUSSIAN_RADIUS; ++i)
        {
            // We omit the normalization factor here for the discrete version and normalize using the sum afterwards
            mBlurParams.Coefficients[i] = (1.0f / sigma) * std::expf(-static_cast<float>(i * i) / twoSigmaSq);

            // We use each entry twice since we only compute one half of the curve
            sum += 2 * mBlurParams.Coefficients[i];
        }

        // The center (index 0) has been counted twice, so we subtract it once
        sum -= mBlurParams.Coefficients[0];

        // We normalize all entries using the sum so that the entire kernel gives us a sum of coefficients = 0
        float normalizationFactor = 1.0f / sum;
        for (Uint i = 0; i <= GAUSSIAN_RADIUS; ++i)
            mBlurParams.Coefficients[i] *= normalizationFactor;
    }
}