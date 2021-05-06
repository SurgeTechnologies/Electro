//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroMaterial.hpp"
#include "Renderer/EGenerator.hpp"

namespace Electro
{
    Material::Material(const Ref<Shader>& shader, const String& nameInShader)
    {
        mShader = shader;
        mBufferName = nameInShader;
        mReflectionData = shader->GetReflectionData(ShaderDomain::PIXEL);
        mTextures.resize(mReflectionData.GetResources().size());
        Allocate(nameInShader);
    }

    void Material::Bind()
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
        mCBuffer = EGenerator::CreateConstantBuffer(mCBufferMemory.GetSize(), shaderBuffer.Binding, DataUsage::DYNAMIC);
    }
}
