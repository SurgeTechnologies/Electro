//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Camera.hpp"
#include "Core/TimeStep.hpp"
#include "Core/Events/KeyEvent.hpp"
#include "Core/Events/MouseEvent.hpp"

namespace Electro
{
    enum class CameraMode
    {
        NONE, FLYCAM, ARCBALL
    };

    class EditorCamera : public Camera
    {
    public:
        EditorCamera() = default;
        EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

        void Focus(const glm::vec3& focusPoint);
        void OnUpdate(Timestep ts);
        void OnEvent(Event& e);

        bool IsActive() const { return mIsActive; }
        void SetActive(bool active) { mIsActive = active; }

        float GetDistance() const { return mDistance; }
        void SetDistance(float distance) { mDistance = distance; }

        const glm::vec3& GetFocalPoint() const { return mFocalPoint; }

        void SetViewportSize(float width, float height) { mViewportWidth = width; mViewportHeight = height; UpdateProjection(); }

        const glm::mat4& GetViewMatrix() const { return mViewMatrix; }
        glm::mat4 GetViewProjection() const { return mProjection * mViewMatrix; }

        glm::vec3 GetUpDirection() const;
        glm::vec3 GetRightDirection() const;
        glm::vec3 GetForwardDirection() const;

        const glm::vec3& GetPosition() const { return mPosition; }

        glm::quat GetOrientation() const;

        float GetPitch() const { return mPitch; }
        float GetYaw() const { return mYaw; }
        float& GetCameraSpeed() { return mSpeed; }
        float GetCameraSpeed() const { return mSpeed; }
    private:
        void UpdateCameraView();
        void UpdateProjection();

        bool OnMouseScroll(MouseScrolledEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnKeyReleased(KeyReleasedEvent& e);

        void MousePan(const glm::vec2& delta);
        void MouseRotate(const glm::vec2& delta);
        void MouseZoom(float delta);

        glm::vec3 CalculatePosition() const;

        Pair<float, float> PanSpeed() const;
        float RotationSpeed() const;
        float ZoomSpeed() const;
    private:
        glm::mat4 mViewMatrix;
        glm::vec3 mPosition, mWorldRotation, mFocalPoint;

        bool mIsActive = false;
        glm::vec2 mInitialMousePosition{};

        float mDistance;
        float mSpeed{ 0.002f };
        float mLastSpeed = 0.0f;

        float mPitch, mYaw;
        float mPitchDelta{}, mYawDelta{};
        glm::vec3 mPositionDelta{};
        glm::vec3 mRightDirection{};

        CameraMode mCameraMode{ CameraMode::ARCBALL };

        float mMinFocusDistance = 100.0f;

        float mViewportWidth = 1280.0f, mViewportHeight = 720.0f;
        float mFOV = 45.0f, mAspectRatio = 1.778f, mNearClip = 0.1f, mFarClip = 1000.0f;
    };

}
