//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Material.hpp"
#include "Renderer/Factory.hpp"

namespace Electro
{
    Material::Material(const Ref<Shader>& shader, const String& nameInShader, const String& name)
        : mShader(shader), mBufferName(nameInShader), mName(name)
    {
        mReflectionData = shader->GetReflectionData(ShaderDomain::Pixel);
        mTextures.resize(mReflectionData.GetResources().size());
        Allocate(nameInShader);
    }

    const void Material::Bind() const
    {
        mShader->Bind();

        for (Uint i = 0; i < mTextures.size(); i++)
        {
            Ref<Texture2D> tex = mTextures[i];
            if (tex)
                mTextures[i]->PSBind(i);
        }

        mCBuffer->SetDynamicData(mCBufferMemory.GetData());
        mCBuffer->PSBind();
    }

    void Material::Allocate(const String& name)
    {
        mCBufferMemory = Buffer();
        const ShaderBuffer& shaderBuffer = mReflectionData.GetBuffer(name);
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
