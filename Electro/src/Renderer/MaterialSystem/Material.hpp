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
    enum class MaterialType
    {
        BuiltIn,
        RenderMaterial
    };

    enum class TextureExtension : int32_t
    {
        Png = 0, Jpg, Tga, Bmp, Psd, Hdr, Pic, Gif
    };

    String TextureExtensionToString(TextureExtension e);
    TextureExtension StringToTextureExtension(const String& s);

    class Material : public Asset
    {
    public:
        Material() = default;
        Material(const Ref<Shader>& shader, const String& nameInShader, const String& pathOrName = "");
        ~Material();

        void Bind() const;
        ShaderReflectionData& GetReflectionData() { return mReflectionData; }
        Ref<Shader>& GetShader() { return mShader; }

        template<typename T>
        void Set(const String& name, const T& value)
        {
            const ShaderBuffer& buffer = mReflectionData.GetBuffer(mBufferName);
            const ShaderBufferMember& member = mReflectionData.GetBufferMember(buffer, name);
            mCBufferMemory.Write((byte*)&value, sizeof(value), member.MemoryOffset);

            if(mMaterialType == MaterialType::RenderMaterial)
                Serialize();
        }

        void Set(const String& name, const Ref<Texture2D>& resource, bool forceTexture = false)
        {
            const String extStr = TextureExtensionToString(mTextureExtension);
            const String texExt = FileSystem::GetExtension(resource->GetPath());
            if(!forceTexture)
            {
                if(extStr != texExt)
                {
                    ELECTRO_ERROR("Cannot set %s! The texture extension doesnt match the selected extension!", name.c_str());
                    ELECTRO_ERROR("Selected extension: %s", extStr.c_str());
                    ELECTRO_ERROR("Texture extension: %s", texExt.c_str());
                    return;
                }
            }
            for (ShaderResource& res : mReflectionData.GetResources())
                if (res.Name == name)
                    mTextures[res.Binding] = resource;

            if(forceTexture)
                if(extStr != texExt)
                    mTextureExtension = StringToTextureExtension(texExt);

            if(mMaterialType == MaterialType::RenderMaterial)
                Serialize();
        }

        template<typename T>
        T& Get(const String& name)
        {
            const ShaderBuffer& buffer = mReflectionData.GetBuffer(mBufferName);
            const ShaderBufferMember& member = mReflectionData.GetBufferMember(buffer, name);
            return mCBufferMemory.Read<T>(member.MemoryOffset);
        }

        Ref<Texture2D> GetTexture2D(const String& name)
        {
            for (ShaderResource& res : mReflectionData.GetResources())
            {
                if (res.Name == name)
                {
                    if (mTextures.size() > 1)
                        return mTextures[res.Binding];
                }
            }
            return Ref<Texture2D>(nullptr);
        }

        void Serialize() override;
        void Deserialize() override;
        TextureExtension GetSelectedTexExtension() const { return mTextureExtension; }

        static Ref<Material> Create(const Ref<Shader>& shader, const String& nameInShader, const String& pathOrName = "");
    private:
        void Allocate();
        void EnsureAllTexturesHaveSameExtension();
    public:
        Vector<Ref<Texture2D>> mTextures;

    private:
        String mBufferName;
        TextureExtension mTextureExtension = TextureExtension::Jpg;
        Ref<Shader> mShader;

        ShaderReflectionData mReflectionData;
        Buffer mCBufferMemory;
        Ref<ConstantBuffer> mCBuffer;
        MaterialType mMaterialType;

        friend class MaterialPanel;
    };
}
