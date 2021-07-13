//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Asset/AssetBase.hpp"
#include "Renderer/Interface/Pipeline.hpp"
#include "Renderer/Interface/Texture.hpp"
#include "Renderer/Interface/ConstantBuffer.hpp"
#include "Renderer/Material.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    class EnvironmentMap : public Asset
    {
    public:
        EnvironmentMap() = default;
        EnvironmentMap(const String& hdrMapPath);
        ~EnvironmentMap() = default;

        // Renders the EnvironmentMap
        void Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);

        // Get the path, from which the Cubemap was loaded
        const String& GetPath() const { return mPath; }

        // Get the underlying cubemap
        const Ref<Cubemap>& GetCubemap() const { return mEnvironmentMap; }

        static Ref<EnvironmentMap> Create(const String& path);
    public:
        float mTextureLOD = 0.0f;
        float mIntensity = 1.0f;
    private:
        String mPath;
        Ref<Shader> mSkyboxShader;
        Ref<Material> mSkyboxMaterial;

        Ref<ConstantBuffer> mSkyboxCBuffer;
        Ref<Cubemap> mEnvironmentMap;
        Ref<Texture2D> mBRDFLUT;
    };
}
