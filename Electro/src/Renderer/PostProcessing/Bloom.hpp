//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "PostProcessingPipeline.hpp"
#include "Renderer/Interface/ConstantBuffer.hpp"
#define GAUSSIAN_RADIUS 39

namespace Electro
{
    struct BlurParams
    {
        float Coefficients[GAUSSIAN_RADIUS + 1];
        int Radius;
        int Direction;
        float GaussianSigma = 15.0f;
        int __Padding = {};
    };

    class Bloom : public IPostProcessMethod
    {
    public:
        virtual void Init(const Ref<Renderbuffer>& target) override;
        virtual void Process() override;
        virtual const Ref<Renderbuffer>& GetOutputRenderBuffer() const override { return mBloomRenderTargets[0]; }

        BlurParams& GetBlurParams() { return mBlurParams; }

        bool GetBloomState() const { return mBloomEnabled; }
        void SetBloomState(bool enabled) { mBloomEnabled = enabled; }

        float GetBloomThreshold() const { return mBloomThreshold; }
        void SetBloomThreshold(float threshold) { mBloomThreshold = threshold; }
    private:
        void CalculateGaussianCoefficients();
    private:
        BlurParams mBlurParams;
        bool mBloomEnabled = true;
        float mBloomThreshold = 1.0f;

        Ref<Renderbuffer> mTarget;
        Ref<Renderbuffer> mBloomRenderTargets[2];

        Ref<Shader> mThresholdDownsampleShader;
        Ref<Shader> mGaussianBlurShader;

        Ref<ConstantBuffer> mBlurParamsCBuffer;
        Ref<ConstantBuffer> mBloomThresholdCBuffer;
    };
}