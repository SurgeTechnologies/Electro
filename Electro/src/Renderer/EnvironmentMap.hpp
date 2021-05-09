//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/Pipeline.hpp"
#include "Renderer/Interface/Texture.hpp"
#include "Renderer/Interface/ConstantBuffer.hpp"
#include "Renderer/MaterialSystem/Material.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    class EnvironmentMap : public IElectroRef
    {
    public:
        EnvironmentMap() = default;
        EnvironmentMap(const String& hdrMapPath);
        ~EnvironmentMap() = default;
        void Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
        String GetPath() { return mEnvironmentMap->GetPath(); }
        Ref<Cubemap>& GetCubemap() { return mEnvironmentMap; }
    public:
        float mTextureLOD = 0;
        float mIntensity = 1;
    private:
        Ref<Shader> mSkyboxShader;
        Ref<Shader> mPBRShader;
        Ref<Material> mSkyboxMaterial;

        Ref<ConstantBuffer> mSkyboxCBuffer;
        Ref<Cubemap> mEnvironmentMap;
        Ref<Texture2D> mBRDFLUT;
    };
}
