//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IRenderPass.hpp"
#include "Renderer/Interface/ConstantBuffer.hpp"

// Cascade Defines
#define NUM_CASCADES 4
#define NUM_FRUSTUM_CORNERS 8

namespace Electro
{
    typedef glm::vec<NUM_CASCADES, float, glm::defaultp> CascadeFloats;

    class ShadowPass : public IRenderPass
    {
    public:
        virtual void Init(RendererData* rendererData) override;
        virtual void Update() override;
        virtual void Shutdown() override;

        E_FORCE_INLINE void ResizeAllShadowMaps(Uint shadowMapResolution)
        {
            mData.ShadowMapResolution = shadowMapResolution;
            for (Ref<Renderbuffer>& shadowMap : mData.ShadowMaps)
                shadowMap->Resize(shadowMapResolution, shadowMapResolution);
        }

        E_FORCE_INLINE void BindAllShadowMaps(Uint slot) const
        {
            for (Uint i = 0; i < NUM_CASCADES; i++)
                mData.ShadowMaps[i]->BindDepthBuffer(slot + i, ShaderDomain::PIXEL);
        }

        E_FORCE_INLINE void UnbindAllShadowMaps(Uint slot) const
        {
            for (Uint i = 0; i < NUM_CASCADES; i++)
                mData.ShadowMaps[i]->UnbindBuffer(slot + i, ShaderDomain::PIXEL);
        }

    protected:
        virtual void* GetInternalDataBlock() override { return &mData; };
    private:
        void CalculateMatricesAndUpdateCBuffer(glm::mat4& view, const glm::mat4& projection, const glm::vec3& normalizedDirection);
    public:
        struct InternalData
        {
            Ref<Shader> ShadowMapShader;
            Ref<ConstantBuffer> ShadowCBuffer;
            Ref<Renderbuffer> ShadowMaps[NUM_CASCADES];

            Uint ShadowMapResolution = 2048;
            float CascadeSplitLambda = 0.91f;
            glm::mat4 ViewProjections[NUM_CASCADES] = {};

            CascadeFloats CascadeSplits = {};
            CascadeFloats CascadeSplitDepths = {};
        };
    private:
        InternalData mData;
        friend class RenderPassManager;
    };
}
