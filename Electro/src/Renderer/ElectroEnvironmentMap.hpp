//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/ElectroPipeline.hpp"
#include "Renderer/Interface/ElectroTexture.hpp"
#include "Renderer/Interface/ElectroConstantBuffer.hpp"
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
        Ref<Shader> mPBRShader;
        Ref<Pipeline> mPipeline;
        Ref<ConstantBuffer> mSkyboxCBuffer;
        Ref<ConstantBuffer> mSkyboxPixelShaderCBuffer;
        Ref<Cubemap> mEnvironmentMap;
        Ref<Texture2D> mBRDFLUT;
    };
}
