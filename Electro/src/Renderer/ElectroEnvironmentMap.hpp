//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroPipeline.hpp"
#include "Renderer/ElectroTexture.hpp"
#include "Renderer/ElectroConstantBuffer.hpp"
#include <glm/glm.hpp>
#include <d3d11.h>

namespace Electro
{
    class EnvironmentMap : public IElectroRef
    {
    public:
        EnvironmentMap() = default;
        EnvironmentMap(const String& hdrMapPath);
        ~EnvironmentMap() = default;
        void Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
        String GetFilePath() { return ""; };
    private:
        Ref<Pipeline> mPipeline;
        Ref<ConstantBuffer> mSkyboxCBuffer;
        ID3D11ShaderResourceView* mSRV;
    };
}