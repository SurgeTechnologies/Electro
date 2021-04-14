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
        ConstantBufferDesc desc;
        desc.Shader = shader;
        desc.Name = "Material";
        desc.InitialData = nullptr;
        desc.Size = sizeof(MaterialCbuffer);
        desc.BindSlot = 2;
        desc.ShaderDomain = ShaderDomain::PIXEL;
        desc.Usage = DataUsage::DYNAMIC;
        mCBuffer = ConstantBuffer::Create(desc);
    }

    void Material::Bind(Uint index)
    {
        mShader->Bind();

        mCBuffer->SetData(&mCBufferData);
    }

    void Material::PushTexture(const Ref<Texture2D>& tex, Uint slot)
    {
         mTextures[slot] = tex;
    }

    void Material::FlipTextures(bool flip)
    {
        for (auto& texture : mTextures)
            if (texture)
                texture->Reload(flip);
    }
}