//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroShader.hpp"
#include "Renderer/ElectroTexture.hpp"
#include "Renderer/ElectroConstantBuffer.hpp"

namespace Electro
{
    struct MaterialCbuffer
    {
        glm::vec3 Albedo = { 1.0f, 1.0f, 1.0f };
        float Metallic = 0.0f;

        float Roughness;
        float AO = 1.0f;
        int AlbedoTexToggle;
        int MetallicTexToggle;

        int AOTexToggle;
        int RoughnessTexToggle;
        glm::vec2 __Padding1;
    };

    class Material : public IElectroRef
    {
    public:
        Material() = default;
        Material(const Ref<Shader>& shader);
        ~Material() = default;

        void Bind(Uint index);
        Ref<Shader>& GetShader() { return mShader; }

        MaterialCbuffer& GetCBufferData() { return mCBufferData; }
        static Ref<Material> Material::Create(const Ref<Shader>& shader);
    public:
        //All maps
        Ref<Texture2D> mAlbedoMap;   //Register 0
        Ref<Texture2D> mNormalMap;   //Register 1
        Ref<Texture2D> mMetallicMap; //Register 2
        Ref<Texture2D> mRoughnessMap;//Register 3
        Ref<Texture2D> mAOMap;       //Register 4
        bool mFlipped;
    private:
        Ref<Shader> mShader;
        MaterialCbuffer mCBufferData;
        Ref<ConstantBuffer> mCBuffer;
    private:
        friend class MaterialPanel;
    };
}