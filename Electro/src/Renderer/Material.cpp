//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Material.hpp"
#include "Asset/AssetManager.hpp"

namespace Electro
{
    String TextureExtensionToString(TextureExtension e)
    {
        switch (e)
        {
            case TextureExtension::Png: return ".png";
            case TextureExtension::Jpg: return ".jpg";
            case TextureExtension::Tga: return ".tga";
            case TextureExtension::Bmp: return ".bmp";
            case TextureExtension::Psd: return ".psd";
            case TextureExtension::Hdr: return ".hdr";
            case TextureExtension::Pic: return ".pic";
            case TextureExtension::Gif: return ".gif";
        }
        return {};
    }

    TextureExtension StringToTextureExtension(const String& s)
    {
        if (s == ".png") return TextureExtension::Png;
        if (s == ".jpg") return TextureExtension::Jpg;
        if (s == ".tga") return TextureExtension::Tga;
        if (s == ".bmp") return TextureExtension::Bmp;
        if (s == ".psd") return TextureExtension::Psd;
        if (s == ".hdr") return TextureExtension::Hdr;
        if (s == ".pic") return TextureExtension::Pic;
        if (s == ".gif") return TextureExtension::Gif;
        return {};
    }

    Material::Material(const Ref<Shader>& shader, const String& nameInShader, const String& pathOrName)
        : mBufferName(nameInShader), mShader(shader)
    {
        if(!FileSystem::ValidatePath(pathOrName))
        {
            SetupAssetBase(pathOrName, AssetType::Material, pathOrName);
            mMaterialType = MaterialType::BuiltIn;
        }
        else
        {
            SetupAssetBase(pathOrName, AssetType::Material);
            mMaterialType = MaterialType::RenderMaterial;
        }

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

        mCBuffer->PSBind();
        mCBuffer->SetDynamicData(mCBufferMemory.GetData());
    }

    bool Material::Serialize()
    {
        EnsureAllTexturesHaveSameExtension();
        std::ofstream out(mPathInDisk, std::ios::binary);
        if (out)
        {
            out.write(static_cast<char*>(mCBufferMemory.GetData()), mCBufferMemory.GetSize());
            for (const Ref<Texture2D>& tex : mTextures)
            {
                if(tex)
                    out.write(tex->GetPath().c_str(), tex->GetPath().size());
                else
                {
                    String extStr = TextureExtensionToString(mTextureExtension);
                    out.write(("Empty" + extStr).c_str(), std::strlen(("Empty" + extStr).c_str()));
                }
            }
            out.close();
            return true;
        }
        return false;
    }

    bool Material::Deserialize()
    {
        std::ifstream in(mPathInDisk);
        if (in)
        {
            //Get the file size
            in.seekg(0, std::ios::end);
            const auto size = in.tellg();
            in.seekg(0, std::ios::beg);

            mCBufferMemory.ZeroMem();
            in.read(static_cast<char*>(mCBufferMemory.GetData()), mCBufferMemory.GetSize());

            if(size != mCBufferMemory.GetSize())
            {
                //Make sure that the cursor is after the mCBufferMemory size
                in.seekg(mCBufferMemory.GetSize());

                //Get the actual vector
                const Vector<char> vec = Vector<char>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
                Vector<String> paths;
                String extStr = TextureExtensionToString(mTextureExtension);
                const size_t n = std::strlen(extStr.c_str());
                for (auto first = std::begin(vec), last = std::end(vec), it = first;
                     (it = std::search(first, last, extStr.c_str(), extStr.c_str() + n)) != last;
                     first = it
                )
                {
                    std::advance(it, n);
                    paths.push_back({first, it });
                }
                for (Uint i = 0; i < paths.size(); i++)
                {
                    if(paths[i] == ("Empty" + extStr))
                        continue;
                    mTextures[i] = Texture2D::Create(paths[i]);
                }
            }
            in.close();
            return true;
        }
        return false;
    }

    void Material::Allocate()
    {
        mCBufferMemory = Buffer();
        const ShaderBuffer& shaderBuffer = mReflectionData.GetBuffer(mBufferName);
        mCBufferMemory.Allocate(shaderBuffer.Size);
        mCBufferMemory.ZeroMem();
        mCBuffer = ConstantBuffer::Create(mCBufferMemory.GetSize(), shaderBuffer.Binding, DataUsage::DYNAMIC);
    }

    void Material::EnsureAllTexturesHaveSameExtension()
    {
        Vector<String> exts;
        for(const Ref<Texture2D>& tex : mTextures)
            if (tex)
                exts.push_back(FileSystem::GetExtension(tex->GetPath()));

        if (!(std::adjacent_find(exts.begin(), exts.end(), std::not_equal_to<>()) == exts.end()))
        {
            Log::Warn("All textures got removed from material because their extension doesn't match with each other! :)");
            mTextures.clear();
        }
    }

    Ref<Material> Material::Create(const Ref<Shader>& shader, const String& nameInShader, const String& pathOrName)
    {
        Ref<Material> result = AssetManager::Get<Material>(AssetManager::GetHandle(pathOrName));
        if (!result)
        {
            result = Ref<Material>::Create(shader, nameInShader, pathOrName);
            AssetManager::Submit<Material>(result);
        }
        return result;
    }
}
