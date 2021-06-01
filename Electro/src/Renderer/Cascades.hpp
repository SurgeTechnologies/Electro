//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Interface/Framebuffer.hpp"
#include <glm/glm.hpp>

#define NUM_CASCADES 3

namespace Electro
{
    class Cascades
    {
    public:
        void Init();
        void CalculateViewProjection(glm::mat4& view, const glm::mat4& projection, const glm::vec3& normalizedDirection);
        void Bind(Uint slot) const;
        void Unbind(Uint slot) const;
        const Ref<Framebuffer>* GetFramebuffers() const { return mShadowMaps; }
        glm::vec4 GetCascadeSplitDepths() { return glm::vec4(mCascadeSplitDepth[0], mCascadeSplitDepth[1], mCascadeSplitDepth[2], 1.0f); }
        const glm::mat4* GetViewProjections() const { return mViewProjections; }
    private:
        Ref<Framebuffer> mShadowMaps[NUM_CASCADES];
        glm::mat4 mViewProjections[NUM_CASCADES] = {};
        float mCascadeSplits[NUM_CASCADES] = {};
        float mCascadeSplitDepth[NUM_CASCADES] = {};
        float mCascadeFarPlaneOffset = 15.0f;
        float mCascadeNearPlaneOffset = -15.0f;
    };
}
