//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/Shader.hpp"
#include "Renderer/Interface/Texture.hpp"
#include "Renderer/Interface/ConstantBuffer.hpp"
#include "ReflectionData.hpp"
#include "Core/Buffer.hpp"

namespace Electro
{
    class Material : public IElectroRef
    {
    public:
        Material() = default;
        Material(const Ref<Shader>& shader, const String& nameInShader, const String& name);
        ~Material() = default;

        const void Bind() const;
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
            for (ShaderResource& res : mReflectionData.GetResources())
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

        Ref<Texture2D>& Get(const String& name)
        {
            for (ShaderResource& res : mReflectionData.GetResources())
                if (res.Name == name)
                    if(mTextures.size() > 1)
                        return mTextures[res.Binding];
            return GetNullTexture();
        }

    private:
        void Allocate(const String& name);
        Ref<Texture2D>& GetNullTexture();
    public:
        Vector<Ref<Texture2D>> mTextures;

    private:
        String mName;
        String mBufferName;
        Ref<Shader> mShader;
        ShaderReflectionData mReflectionData;
        Buffer mCBufferMemory;
        Ref<ConstantBuffer> mCBuffer;
    private:
        friend class MaterialPanel;
    };
}
