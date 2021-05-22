//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Material.hpp"
#include "Core/FileSystem.hpp"
#include "Renderer/Factory.hpp"

namespace Electro
{
    Material::Material(const Ref<Shader>& shader, const String& nameInShader, const String& path)
        : mBufferName(nameInShader), mShader(shader)
    {
        if(!FileSystem::ValidatePath(path))
            SetupAssetBase("", AssetType::Material, path);
        else
            SetupAssetBase(path, AssetType::Material);

        mReflectionData = shader->GetReflectionData(ShaderDomain::Pixel);
        mTextures.resize(mReflectionData.GetResources().size());
        Allocate();
    }

    Material::~Material()
    {
        if (mCBufferMemory)
            mCBufferMemory.Release();
    }

    void Material::Bind() const
    {
        mShader->Bind();

        for (Uint i = 0; i < mTextures.size(); i++)
        {
            const Ref<Texture2D>& tex = mTextures[i];
            if (tex)
                mTextures[i]->PSBind(i);
        }

        //Upload the data to GPU
        mCBuffer->SetDynamicData(mCBufferMemory.GetData());
        mCBuffer->PSBind();
    }

    void Material::Allocate()
    {
        mCBufferMemory = Buffer();
        const ShaderBuffer& shaderBuffer = mReflectionData.GetBuffer(mBufferName);
        mCBufferMemory.Allocate(shaderBuffer.Size);
        mCBufferMemory.ZeroMem();
        mCBuffer = Factory::CreateConstantBuffer(mCBufferMemory.GetSize(), shaderBuffer.Binding, DataUsage::DYNAMIC);
    }

    Ref<Texture2D> dummyTexture = nullptr;
    Ref<Texture2D>& Material::GetNullTexture()
    {
        return dummyTexture;
    }

}
