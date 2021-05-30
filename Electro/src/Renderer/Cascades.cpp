//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Cascades.hpp"
#include "Factory.hpp"
#include "Math/BoundingBox.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "RendererDebug.hpp"
#include "Platform/DX11/DX11Internal.hpp"

namespace Electro
{
    void Cascades::Init()
    {
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::R32_TYPELESS };
        fbSpec.Width = 4096;
        fbSpec.Height = 4096;
        fbSpec.SwapChainTarget = false;

        for(Ref<Framebuffer>& shadowMap : mShadowMaps)
            shadowMap = Factory::CreateFramebuffer(fbSpec);
    }

    void Cascades::CalculateCascadeEnds(float nearClip, float distance)
    {
        //for (Uint i = 0; i < NUM_CASCADES; i++)
        //{
        //    float idk = i / float(NUM_CASCADES);
        //    float log = nearClip * powf(distance / nearClip, idk);
        //    float uniform = nearClip + (distance - nearClip) * idk;
        //    mCascadeEnds[i] = log * 0.5f + uniform * 0.5f;
        //}
        mCascadeEnds[0] = nearClip;
        mCascadeEnds[1] = 30.0f;
        mCascadeEnds[2] = 50.0f;
        mCascadeEnds[3] = 300;

        //for(Uint i = 0; i < NUM_CASCADES + 1; i++)
        //    ELECTRO_INFO("mCascadeEnds[%i] = %f", i, mCascadeEnds[i]);
    }

    inline glm::vec4 GetColor(Uint cascade)
    {
        if(cascade == 0)
            return { 1.0, 0.0, 0.0f, 1.0f };
        if(cascade == 1)
            return { 0.0, 1.0, 0.0f, 1.0f };
        if(cascade == 2)
            return { 0.0, 0.0, 1.0f, 1.0f };
        if(cascade == 3)
            return { 1.0, 1.0, 0.0f, 1.0f };
        return {};
    }

    void Cascades::CalculateViewProjection(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& direction)
    {
        glm::mat4 viewProjection = projection * view;
        glm::mat4 inverseViewProjection = glm::inverse(viewProjection);
        glm::mat4 shadowView = glm::lookAt(-glm::normalize(direction), { 0.0f, 0.0f, 0.0f }, { 0.001f, 1.0f, 0.001f });

        for (Uint cascade = 0; cascade < NUM_CASCADES; cascade++)
        {
            // Get the near and far plane for this cascade
            float nearPlane = mCascadeEnds[cascade];
            float farPlane = mCascadeEnds[cascade + 1];

            //View matrix go brrr
            glm::vec4 temp = projection * glm::vec4(0.0f, 0.0f, nearPlane, 1.0f);
            const float nearClipSpace = temp.z / temp.w;
            temp = projection * glm::vec4(0.0f, 0.0f, farPlane, 1.0f);
            const float farClipSpace = temp.z / temp.w;

            glm::vec4 frustumCorners[8] =
            {
                {-1.0f, -1.0f, nearClipSpace, 1.0f},
                {-1.0f,  1.0f, nearClipSpace, 1.0f},
                { 1.0f,  1.0f, nearClipSpace, 1.0f},
                { 1.0f, -1.0f, nearClipSpace, 1.0f},
                {-1.0f, -1.0f,  farClipSpace, 1.0f},
                {-1.0f,  1.0f,  farClipSpace, 1.0f},
                { 1.0f,  1.0f,  farClipSpace, 1.0f},
                { 1.0f, -1.0f,  farClipSpace, 1.0f},
            };

            // Initialize min and max corners for AABB
            BoundingBox aabb;
            aabb.Reset();

            // Transform frustum corners to world space using the inverse view projection matrix
            for (glm::vec4& frustumCorner : frustumCorners)
            {
                frustumCorner = inverseViewProjection * frustumCorner;
                frustumCorner /= frustumCorner.w;
            }

            RendererDebug::BeginScene(viewProjection);
            RendererDebug::SubmitCameraFrustum(frustumCorners, viewProjection, GetColor(cascade));
            RendererDebug::EndScene();

            // Transform the corners to light space so we can calculate the AABB for the projection matrix
            for (glm::vec4& frustumCorner : frustumCorners)
            {
                frustumCorner = shadowView * frustumCorner;
                aabb.Min = glm::min(glm::vec3(frustumCorner), aabb.Min);
                aabb.Max = glm::max(glm::vec3(frustumCorner), aabb.Max);
            }

            // Create projection matrix using the bounding box
            const glm::vec3& min = aabb.Min;
            const glm::vec3& max = aabb.Max;
            glm::mat4 shadowProj = glm::ortho(min.x, max.x, min.y, max.y, min.z, max.z);
            mViewProjections[cascade] = shadowProj * shadowView;
        }
    }

    //TODO: Abstract these out to DX11Framebuffer
    void Cascades::Bind(Uint slot) const
    {
        for(Uint i = 0; i < NUM_CASCADES; i++)
        {
            ID3D11ShaderResourceView* dsrv = static_cast<ID3D11ShaderResourceView*>(mShadowMaps[i]->GetDepthAttachmentID());
            DX11Internal::GetDeviceContext()->PSSetShaderResources(slot + i, 1, &dsrv);
        }
    }
    void Cascades::Unbind(Uint slot) const
    {
        ID3D11ShaderResourceView* null = nullptr;
        for(Uint i = 0; i < NUM_CASCADES; i++)
            DX11Internal::GetDeviceContext()->PSSetShaderResources(slot + i, 1, &null);
    }
}
