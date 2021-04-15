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

        if (mCBufferData.AlbedoTexToggle && mAlbedoMap)
            mAlbedoMap->Bind(0);

        if (mCBufferData.NormalTexToggle && mNormalMap)
            mNormalMap->Bind(1);

        if (mCBufferData.MetallicTexToggle && mMetallicMap)
            mMetallicMap->Bind(2);

        if (mCBufferData.RoughnessTexToggle && mRoughnessMap)
            mRoughnessMap->Bind(3);

        if (mCBufferData.AOTexToggle && mAOMap)
            mAOMap->Bind(4);

        mCBuffer->SetData(&mCBufferData);
    }
}
