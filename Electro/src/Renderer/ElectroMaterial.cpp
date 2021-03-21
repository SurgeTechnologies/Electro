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
        :m_Shader(shader)
    {
        m_CBuffer = ConstantBuffer::Create(shader, "Material", nullptr, sizeof(MaterialCbuffer), 2, ShaderDomain::PIXEL, DataUsage::DYNAMIC);
    }

    void Material::Bind(Uint index)
    {
        m_Shader->Bind();
        m_CBufferData.AlbedoTexToggle = m_AlbedoTexToggle;
        m_CBufferData.Color = m_Color;
        m_CBufferData.Shininess = m_Shininess;

        if (m_CBufferData.AlbedoTexToggle == 1)
        {
            for (size_t i = 0; i < m_Textures.size(); i++)
            {
                auto& texture = m_Textures[index];
                if (texture)
                    texture->Bind(i);
            }
        }

        m_CBuffer->SetData(&m_CBufferData);

        //TODO: Remove (Required for GLSL only)
        //m_Shader->SetInt("u_DiffuseTexture", index);
    }

    void Material::PushTexture(const Ref<Texture2D>& tex, Uint slot)
    {
         m_Textures[slot] = tex;
    }

    void Material::SetDiffuseTexToggle(bool value)
    {
        m_AlbedoTexToggle = value;
        if (m_CBuffer)
            m_CBuffer->Bind();
    }

    void Material::FlipTextures(bool flip)
    {
        for (auto& texture : m_Textures)
            if (texture)
                texture->Reload(flip);
    }
}