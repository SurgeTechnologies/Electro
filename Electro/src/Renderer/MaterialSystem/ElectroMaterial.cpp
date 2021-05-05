//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroMaterial.hpp"
#include "Renderer/EGenerator.hpp"

namespace Electro
{
    Ref<Material> Material::Create(const Ref<Shader>& shader)
    {
        return Ref<Material>::Create(shader);
    }

    Material::Material(const Ref<Shader>& shader)
    {
        mShader = shader;
        mReflectionData = shader->GetReflectionData(ShaderDomain::PIXEL);
        mCBuffer = EGenerator::CreateConstantBuffer(sizeof(MaterialCbuffer), 2, DataUsage::DYNAMIC);
    }

    void Material::Bind(Uint index)
    {
        mShader->Bind();

        if (mCBufferData.AlbedoTexToggle && mAlbedoMap.Data1)
            mAlbedoMap.Data1->PSBind(mAlbedoMap.Data2);

        if (mCBufferData.NormalTexToggle && mNormalMap.Data1)
            mNormalMap.Data1->PSBind(mNormalMap.Data2);

        if (mCBufferData.MetallicTexToggle && mMetallicMap.Data1)
            mMetallicMap.Data1->PSBind(mMetallicMap.Data2);

        if (mCBufferData.RoughnessTexToggle && mRoughnessMap.Data1)
            mRoughnessMap.Data1->PSBind(mRoughnessMap.Data2);

        if (mCBufferData.AOTexToggle && mAOMap.Data1)
            mAOMap.Data1->PSBind(mAOMap.Data2);

        mCBuffer->SetDynamicData(&mCBufferData);
        mCBuffer->PSBind();
    }
}
