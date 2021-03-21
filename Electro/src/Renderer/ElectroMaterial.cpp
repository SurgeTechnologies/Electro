//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroMaterial.hpp"

namespace Electro
{
    Ref<Material> Material::Create(const Ref<Shader>& shader)
    {
        return Ref<Material>::Create(shader);
    }

    Material::Material(const Ref<Shader>& shader)
        :mShader(shader)
    {
        mCBuffer = ConstantBuffer::Create(shader, "Material", nullptr, sizeof(MaterialCbuffer), 2, ShaderDomain::PIXEL, DataUsage::DYNAMIC);
    }

    void Material::Bind(Uint index)
    {
        mShader->Bind();
        mCBufferData.AlbedoTexToggle = mAlbedoTexToggle;
        mCBufferData.Color = mColor;
        mCBufferData.Shininess = mShininess;

        if (mCBufferData.AlbedoTexToggle == 1)
        {
            for (size_t i = 0; i < mTextures.size(); i++)
            {
                auto& texture = mTextures[index];
                if (texture)
                    texture->Bind(i);
            }
        }

        mCBuffer->SetData(&mCBufferData);

        //TODO: Remove (Required for GLSL only)
        //m_Shader->SetInt("u_DiffuseTexture", index);
    }

    void Material::PushTexture(const Ref<Texture2D>& tex, Uint slot)
    {
         mTextures[slot] = tex;
    }

    void Material::SetDiffuseTexToggle(bool value)
    {
        mAlbedoTexToggle = value;
        if (mCBuffer)
            mCBuffer->Bind();
    }

    void Material::FlipTextures(bool flip)
    {
        for (auto& texture : mTextures)
            if (texture)
                texture->Reload(flip);
    }
}