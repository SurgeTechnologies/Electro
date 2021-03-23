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
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        int AlbedoTexToggle = 0;
        float Shininess = 32.0f;
        glm::vec3 __Padding;
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

        glm::vec3& GetColor() { return mColor; }
        void SetColor(const glm::vec3& color) { mColor = color; }

        void SetDiffuseTexToggle(bool value);
        bool GetDiffuseTexToggle() { return mAlbedoTexToggle; }
        void FlipTextures(bool flip);
        static Ref<Material> Material::Create(const Ref<Shader>& shader);
    public:
        float mShininess = 32.0f;
        glm::vec3 mColor;
        bool mAlbedoTexToggle;
        bool mFlipped;
    private:
        Ref<Shader> mShader;
        Vector<Ref<Texture2D>> mTextures;
        MaterialCbuffer mCBufferData;
        Ref<ConstantBuffer> mCBuffer;
    };
}