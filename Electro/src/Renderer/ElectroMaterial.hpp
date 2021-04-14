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
        float Roughness = 0.0f;
        float AO = 1.0f;
        glm::vec2 __Padding;
    };

    class Material : public IElectroRef
    {
    public:
        Material() = default;
        Material(const Ref<Shader>& shader);
        ~Material() = default;

        void Bind(Uint index);
        Ref<Shader>& GetShader() { return mShader; }

        Vector<Ref<Texture2D>>& GetTextures() { return mTextures; }
        void PushTexture(const Ref<Texture2D>& tex, Uint slot = 0);

        glm::vec3 GetColor() { return mCBufferData.Albedo; }
        void SetColor(const glm::vec3& color) { mCBufferData.Albedo = color; }

        float GetMetallic() { return mCBufferData.Metallic; }
        void SetMetallic(const float metallic) { mCBufferData.Metallic = metallic; }

        float GetRoughness() { return mCBufferData.Roughness; }
        void SetRoughness(const float roughness) { mCBufferData.Roughness = roughness; }

        float GetAO() { return mCBufferData.AO; }
        void SetAO(const float ao) { mCBufferData.AO = ao; }

        void FlipTextures(bool flip);
        static Ref<Material> Material::Create(const Ref<Shader>& shader);
    public:
        bool mFlipped;
    private:
        Ref<Shader> mShader;
        Vector<Ref<Texture2D>> mTextures;
        MaterialCbuffer mCBufferData;
        Ref<ConstantBuffer> mCBuffer;
    private:
        friend class MaterialPanel;
    };
}