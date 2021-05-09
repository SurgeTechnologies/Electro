//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Camera/Camera.hpp"

namespace Electro
{
    class SceneCamera : public Camera
    {
    public:
        enum class ProjectionType { Perspective = 0, Orthographic = 1 };
    public:
        SceneCamera();
        virtual ~SceneCamera() = default;

        void SetPerspective(float verticalFOV, float nearClip, float farClip);
        void SetOrthographic(float size, float nearClip, float farClip);
        void SetViewportSize(Uint width, Uint height);
        float GetAspectRatio() { return mAspectRatio; }
        //Perspective
        float GetPerspectiveVerticalFOV() const { return mPerspectiveFOV; }
        void SetPerspectiveVerticalFOV(float verticalFov) { mPerspectiveFOV = verticalFov; RecalculateProjection(); }

        float GetPerspectiveNearClip() const { return mPerspectiveNear; }
        void SetPerspectiveNearClip(float nearClip) { mPerspectiveNear = nearClip; RecalculateProjection(); }

        float GetPerspectiveFarClip() const { return mPerspectiveFar; }
        void SetPerspectiveFarClip(float farClip) { mPerspectiveFar = farClip; RecalculateProjection(); }


        //Orthographic
        float GetOrthographicSize() const { return mOrthographicSize; }
        void SetOrthographicSize(float size) { mOrthographicSize = size; RecalculateProjection(); }

        float GetOrthographicNearClip() const { return mOrthographicNear; }
        void SetOrthographicNearClip(float nearClip) { mOrthographicNear = nearClip; RecalculateProjection(); }

        float GetOrthographicFarClip() const { return mOrthographicFar; }
        void SetOrthographicFarClip(float farClip) { mOrthographicFar = farClip; RecalculateProjection(); }

        ProjectionType GetProjectionType() const { return mProjectionType; }
        void SetProjectionType(ProjectionType type) { mProjectionType = type; RecalculateProjection(); }
    private:
        void RecalculateProjection();
    public:
        ProjectionType mProjectionType = ProjectionType::Perspective;
    private:
        float mPerspectiveFOV = glm::radians(45.0f);
        float mPerspectiveNear = 0.01f, mPerspectiveFar = 1000.0f;

        float mOrthographicSize = 10.0f;
        float mOrthographicNear = -1.0f, mOrthographicFar = 1.0f;
        float mAspectRatio = 0.0f;
    };
}
