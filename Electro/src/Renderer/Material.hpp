//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/Shader.hpp"
#include "Renderer/Interface/Texture.hpp"
#include "Renderer/Interface/ConstantBuffer.hpp"
#include "ReflectionData.hpp"
#include "Core/Buffer.hpp"
#include "Core/FileSystem.hpp"

namespace Electro
{
    class Material : public IElectroRef
    {
    public:
        Material() = default;
        Material(const Ref<Shader>& shader, const String& nameInShader, const String& name = "");
        ~Material();

        void Bind() const;
        ShaderReflectionData& GetReflectionData() { return mReflectionData; }
        Ref<Shader>& GetShader() { return mShader; }
        const String& GetName() const { return mName; }

        template<typename T>
        void Set(const String& name, const T& value)
        {
            const ShaderBuffer& buffer = mReflectionData.GetBuffer(mBufferName);
            const ShaderBufferMember& member = mReflectionData.GetBufferMember(buffer, name);
            mCBufferMemory.Write((byte*)&value, sizeof(value), member.MemoryOffset);
        }

        void Set(const String& name, const Ref<Texture2D>& resource)
        {
            for (const ShaderResource& res : mReflectionData.GetResources())
                if (res.Name == name)
                    mTextures[res.Binding] = resource;
        }

        template<typename T>
        T& Get(const String& name)
        {
            const ShaderBuffer& buffer = mReflectionData.GetBuffer(mBufferName);
            const ShaderBufferMember& member = mReflectionData.GetBufferMember(buffer, name);
            return mCBufferMemory.Read<T>(member.MemoryOffset);
        }

        Ref<Texture2D>& GetTexture2D(const String& name);
        void RemoveTexture2D(const String& name);

        static Ref<Material> Create(const Ref<Shader>& shader, const String& nameInShader, const String& name);
    private:
        void Allocate();
    public:
        Vector<Ref<Texture2D>> mTextures;
    private:
        String mName;
        String mBufferName;
        Ref<Shader> mShader;

        ShaderReflectionData mReflectionData;
        Buffer mCBufferMemory;
        Ref<ConstantBuffer> mCBuffer;
    };
}
