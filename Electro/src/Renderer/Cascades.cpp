//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Cascades.hpp"
#include "Factory.hpp"
#include "Math/BoundingBox.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

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
        for (Uint i = 0; i < NUM_CASCADES; i++)
        {
            const float idm = i / float(NUM_CASCADES);
            const float log = nearClip * powf(distance / nearClip, idm);
            const float uniform = nearClip + (distance - nearClip) * idm;
            mCascadeEnds[i] = log * 0.5f + uniform * 0.5f;
        }
        mCascadeEnds[0] = nearClip;
        mCascadeEnds[NUM_CASCADES] = distance;

        //for(Uint i = 0; i < NUM_CASCADES + 1; i++)
        //    ELECTRO_INFO("mCascadeEnds[%i] = %f", i, mCascadeEnds[i]);
    }

    glm::vec4 GetColor(Uint cascade)
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
        const glm::mat4 viewProjection = view * projection;
        glm::mat4 inverseViewProjection = glm::inverse(viewProjection);

        //Create the view matrix
        glm::vec3 dir = glm::normalize(direction);
        const glm::vec3 cross = glm::normalize(glm::cross({ 0.0f, 1.0f, 0.0f }, dir));
        const glm::vec3 up = glm::normalize(glm::cross(dir, cross));
        const glm::mat4 shadowView = glm::lookAt(dir, { 0.0f, 0.0f, 0.0f }, up);

        //Loop through cascade distances and calculate the matrices
        for (Uint cascade = 0; cascade < NUM_CASCADES; cascade++)
        {
            //Clip space: The space after multiplying with the projection matrix
            //Get the near and the far plane of the current cascade
            const float nearPlane = mCascadeEnds[cascade];
            const float farPlane = mCascadeEnds[cascade + 1];

            //Use the projection matrix to get the clip space z of each plane
            glm::vec4 temp = projection * glm::vec4(0.0f, 0.0f, nearPlane, 1.0f);
            const float nearClipSpace = temp.z / temp.w;

            temp = projection * glm::vec4(0.0f, 0.0f, farPlane, 1.0f);
            const float farClipSpace = temp.z / temp.w;

            //Store clip space frustum corners
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

            BoundingBox boundingBox = {};
            boundingBox.Reset();

            for (glm::vec4& frustumCorner : frustumCorners)
            {
                //Transform frustum corners to world space using the inverse view projection matrix
                frustumCorner = inverseViewProjection * frustumCorner;
                //frustumCorner /= frustumCorner.w;

                //Transform the corner to light space so we can calculate the AABB for the projection matrix
                frustumCorner = shadowView * frustumCorner;

                //Calculate the min max for the AABB
                glm::vec3 crnr = frustumCorner;
                boundingBox.Min = glm::min(boundingBox.Min, crnr);
                boundingBox.Max = glm::max(boundingBox.Max, crnr);
            }

            boundingBox.Min = glm::round(boundingBox.Min);
            boundingBox.Max = glm::round(boundingBox.Max);

            RendererDebug::BeginScene(viewProjection);
            RendererDebug::DrawAABB(boundingBox, shadowView, GetColor(cascade));
            RendererDebug::EndScene();

            //Create projection matrix using the bounding box. Store the shadow matrix
            glm::vec3& min = boundingBox.Min;
            glm::vec3& max = boundingBox.Max;

            glm::mat4 shadowProjection = glm::ortho(min.x, max.x, min.y, max.y, min.z - 100, max.z);
            mViewProjections[cascade] = shadowView * shadowProjection;
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
