//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroMaterial.hpp"
#include "EDevice/EDevice.hpp"

namespace Electro
{
    Ref<Material> Material::Create(const Ref<Shader>& shader)
    {
        return Ref<Material>::Create(shader);
    }

    Material::Material(const Ref<Shader>& shader)
        :mShader(shader)
    {
        mCBuffer = EDevice::CreateConstantBuffer(sizeof(MaterialCbuffer), 2, ShaderDomain::PIXEL, DataUsage::DYNAMIC);
    }

    void Material::Bind(Uint index)
    {
        mShader->Bind();

        if (mCBufferData.AlbedoTexToggle && mAlbedoMap)
            mAlbedoMap->PSBind(0);

        if (mCBufferData.NormalTexToggle && mNormalMap)
            mNormalMap->PSBind(1);

        if (mCBufferData.MetallicTexToggle && mMetallicMap)
            mMetallicMap->PSBind(2);

        if (mCBufferData.RoughnessTexToggle && mRoughnessMap)
            mRoughnessMap->PSBind(3);

        if (mCBufferData.AOTexToggle && mAOMap)
            mAOMap->PSBind(4);

        mCBuffer->SetData(&mCBufferData);
    }
}
