//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Interface/Framebuffer.hpp"
#include "Interface/ConstantBuffer.hpp"
#include <glm/glm.hpp>

// Cascade Defines
#define NUM_CASCADES 4
#define NUM_FRUSTUM_CORNERS 8

namespace Electro
{
    typedef glm::vec<NUM_CASCADES, float, glm::defaultp> CascadeFloats;

    class Shadows
    {
    public:
        void Init();
        void CalculateMatricesAndSetShadowCBufferData(glm::mat4& view, const glm::mat4& projection, const glm::vec3& normalizedDirection);
        void Bind(Uint slot) const;
        void Unbind(Uint slot) const;
        void Resize(Uint shadowMapResolution);
        void SetCascadeSplitLambda(float lambda) { mCascadeSplitLambda = lambda; }

        const Ref<Framebuffer>* GetFramebuffers() const { return mShadowMaps; }
        const glm::vec4 GetCascadeSplitDepths() const { return mCascadeSplitDepths; }
        const glm::mat4* GetViewProjections() const { return mViewProjections; }
        const Uint GetShadowMapResolution() const { return mShadowMapResolution; }
        const float GetCascadeSplitLambda() const { return mCascadeSplitLambda; }
    private:
        glm::vec4 GetColor(Uint cascade); // Debug only
    private:
        Uint mShadowMapResolution = 2048;
        float mCascadeSplitLambda = 0.91f;
        Ref<Framebuffer> mShadowMaps[NUM_CASCADES];
        glm::mat4 mViewProjections[NUM_CASCADES] = {};

        Ref<ConstantBuffer> mShadowCBuffer;
        CascadeFloats mCascadeSplits = {};
        CascadeFloats mCascadeSplitDepths = {};
        friend class Renderer;
    };
}
