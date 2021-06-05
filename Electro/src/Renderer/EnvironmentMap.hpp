//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Asset/AssetBase.hpp"
#include "Renderer/Interface/Pipeline.hpp"
#include "Renderer/Interface/Texture.hpp"
#include "Renderer/Interface/ConstantBuffer.hpp"
#include "Renderer/MaterialSystem/Material.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    class EnvironmentMap : public Asset
    {
    public:
        EnvironmentMap() = default;
        EnvironmentMap(const String& hdrMapPath);
        ~EnvironmentMap() = default;
        void Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
        const Ref<Cubemap>& GetCubemap() const { return mEnvironmentMap; }
    public:
        float mTextureLOD = 0.0f;
        float mIntensity = 1.0f;
    private:
        Ref<Shader> mSkyboxShader;
        Ref<Shader> mPBRShader;
        Ref<Material> mSkyboxMaterial;

        Ref<ConstantBuffer> mSkyboxCBuffer;
        Ref<Cubemap> mEnvironmentMap;
        Ref<Texture2D> mBRDFLUT;
    };
}
