//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Camera.hpp"
#include "Core/Timestep.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Events/MouseEvent.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    class EditorCamera : public Camera
    {
    public:
        EditorCamera() = default;
        EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

        void OnUpdate(Timestep ts);
        void OnEvent(Event& e);

        float GetDistance() const { return mDistance; }
        void SetDistance(const float distance) { mDistance = distance; }

        void SetViewportSize(const float width, const float height) { mViewportWidth = width; mViewportHeight = height; UpdateProjection(); }
        void Focus(const glm::vec3& focusPoint);

        const glm::mat4& GetViewMatrix() const { return mViewMatrix; }
        glm::mat4& GetViewMatrix() { return mViewMatrix; }
        glm::mat4 GetViewProjection() const { return mProjection * mViewMatrix; }

        glm::vec3 GetUpDirection() const;
        glm::vec3 GetRightDirection() const;
        glm::vec3 GetForwardDirection() const;
        const glm::vec3& GetPosition() const { return mPosition; }
        glm::quat GetOrientation() const;

        float GetPitch() const { return mPitch; }
        float GetYaw() const { return mYaw; }
    private:
        void UpdateProjection();
        void UpdateView();

        bool OnMouseScroll(MouseScrolledEvent& e);

        void MousePan(const glm::vec2& delta);
        void MouseRotate(const glm::vec2& delta);
        void MouseZoom(float delta);
        glm::vec3 CalculatePosition() const;

        std::pair<float, float> PanSpeed() const;
        float RotationSpeed() const;
        float ZoomSpeed() const;
    private:
        float mFOV = 45.0f, mAspectRatio = 1.778f, mNearClip = 0.1f, mFarClip = 1000.0f;

        glm::mat4 mViewMatrix;
        glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };
        glm::vec3 mFocalPoint = { 0.0f, 0.0f, 0.0f };

        glm::vec2 mInitialMousePosition = { 0.0f, 0.0f };

        float mDistance = 10.0f;
        float mPitch = 0.0f, mYaw = 0.0f;
        float mMinFocusDistance = 100.0f;
        float mViewportWidth = 1280, mViewportHeight = 720;
        friend class Renderer;
    };
}
