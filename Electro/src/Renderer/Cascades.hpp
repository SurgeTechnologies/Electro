//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Interface/Framebuffer.hpp"
#include "Interface/ConstantBuffer.hpp"
#include <glm/glm.hpp>

// Cascade Defines
#define NUM_CASCADES 4
#define NUM_FRUSTUM_CORNERS 8
#define CASCADE_SPLIT_LAMBDA 0.91f
#define SHADOW_MAP_RESOLUTION 4096

namespace Electro
{
    typedef glm::vec<NUM_CASCADES, float, glm::defaultp> CascadeFloats;

    class Cascades
    {
    public:
        void Init();
        void CalculateMatricesAndSetShadowCBufferData(glm::mat4& view, const glm::mat4& projection, const glm::vec3& normalizedDirection);
        void Bind(Uint slot) const;
        void Unbind(Uint slot) const;
        const Ref<Framebuffer>* GetFramebuffers() const { return mShadowMaps; }
        glm::vec4 GetCascadeSplitDepths() { return mCascadeSplitDepths; }
        const glm::mat4* GetViewProjections() const { return mViewProjections; }
    private:
        glm::vec4 GetColor(Uint cascade); //Debug only
    private:
        Ref<Framebuffer> mShadowMaps[NUM_CASCADES];
        glm::mat4 mViewProjections[NUM_CASCADES] = {};

        Ref<ConstantBuffer> mShadowCBuffer;

        CascadeFloats mCascadeSplits = {};
        CascadeFloats mCascadeSplitDepths = {};
        friend class Renderer;
    };
}
