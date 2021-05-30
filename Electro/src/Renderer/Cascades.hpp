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
        void CalculateCascadeEnds(float nearClip, float distance);
        void CalculateViewProjection(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& direction);
        void Bind(Uint slot) const;
        void Unbind(Uint slot) const;
        E_NODISCARD const Ref<Framebuffer>* GetFramebuffers() const { return  mShadowMaps; }
        float* GetCascadeEnds() { return  mCascadeEnds; }
        E_NODISCARD const glm::mat4* GetViewProjections() const { return mViewProjections; }
    private:
        Ref<Framebuffer> mShadowMaps[NUM_CASCADES];
        glm::mat4 mViewProjections[NUM_CASCADES] = {};
        float mCascadeEnds[NUM_CASCADES + 1] = {};
        float mCascadeFarPlaneOffset = 15.0f;
        float mCascadeNearPlaneOffset = -15.0f;
    };
}
