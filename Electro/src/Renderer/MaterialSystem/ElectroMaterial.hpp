//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/ElectroShader.hpp"
#include "Renderer/Interface/ElectroTexture.hpp"
#include "Renderer/Interface/ElectroConstantBuffer.hpp"
#include "ElectroReflectionData.hpp"
#include "Core/ElectroBuffer.hpp"

namespace Electro
{
    class Material : public IElectroRef
    {
    public:
        Material() = default;
        Material(const Ref<Shader>& shader, const String& nameInShader);
        ~Material() = default;

        void Bind();

        template<typename T>
        void Set(const String& name, const T& value)
        {
            Pair<String, String>& splittedName = SplitName(name);
            const ShaderBuffer& buffer = mReflectionData.GetBuffer(splittedName.Data1);
            const ShaderBufferMember& member = mReflectionData.GetBufferMember(buffer, splittedName.Data2);
            mCBufferMemory.Write((byte*)&value, sizeof(value), member.MemoryOffset);
        }

        void Set(const String& name, const Ref<Texture2D>& resource)
        {
            for (ShaderResource& res : mReflectionData.GetResources())
                if (res.Name == name)
                    mTextures[res.Binding] = resource;
        }

        template<typename T>
        T& Get(const String& name)
        {
            Pair<String, String>& splittedName = SplitName(name);
            const ShaderBuffer& buffer = mReflectionData.GetBuffer(splittedName.Data1);
            const ShaderBufferMember& member = mReflectionData.GetBufferMember(buffer, splittedName.Data2);
            return mCBufferMemory.Read<T>(member.MemoryOffset);
        }

        Ref<Texture2D>& Get(const String& name)
        {
            for (ShaderResource& res : mReflectionData.GetResources())
                if (res.Name == name)
                    return mTextures[res.Binding];
        }

        Ref<Shader>& GetShader() { return mShader; }
        static Ref<Material> Material::Create(const Ref<Shader>& shader, const String& nameInShader);
    private:
        Pair<String, String> SplitName(const String& name);
        void Allocate(const String& name);
    public:
        Vector<Ref<Texture2D>> mTextures;

    private:
        Ref<Shader> mShader;
        ShaderReflectionData mReflectionData;
        Buffer mCBufferMemory;
        Ref<ConstantBuffer> mCBuffer;
    private:
        friend class MaterialPanel;
    };
}
