//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <glm/glm.hpp>

namespace Electro
{
    class Camera
    {
    public:
        Camera() = default;
        Camera(const glm::mat4& projection)
            : mProjection(projection) {}
        virtual ~Camera() = default;

        E_NODISCARD const glm::mat4& GetProjection() const { return mProjection; }
        E_NODISCARD glm::vec3* GetFrustumPoints(const glm::mat4& cameraTransform)
        {
            const glm::mat4 inv = (cameraTransform * glm::inverse(mProjection));
            glm::vec4 frustumCorners[8] =
            {
                //Near face
                {  1.0f,  1.0f, -1.0f, 1.0f },
                { -1.0f,  1.0f, -1.0f, 1.0f },
                {  1.0f, -1.0f, -1.0f, 1.0f },
                { -1.0f, -1.0f, -1.0f, 1.0f },

                //Far face
                {  1.0f,  1.0f, 1.0f, 1.0f },
                { -1.0f,  1.0f, 1.0f, 1.0f },
                {  1.0f, -1.0f, 1.0f, 1.0f },
                { -1.0f, -1.0f, 1.0f, 1.0f },
            };

            for (Uint i = 0; i < 8; i++)
            {
                const glm::vec4 ff = inv * frustumCorners[i];
                mFrustumPoints[i].x = ff.x / ff.w;
                mFrustumPoints[i].y = ff.y / ff.w;
                mFrustumPoints[i].z = ff.z / ff.w;
            }
            return mFrustumPoints;
        }
    protected:
        glm::mat4 mProjection = glm::mat4(1.0f);
        glm::vec3 mFrustumPoints[8];
    };
}
