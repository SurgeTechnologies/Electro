//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/ElectroShader.hpp"
#include "Renderer/Interface/ElectroTexture.hpp"
#include "Renderer/Interface/ElectroConstantBuffer.hpp"
#include "ElectroReflectionData.hpp"

namespace Electro
{
    struct MaterialCbuffer
    {
        glm::vec3 Albedo = { 1.0f, 1.0f, 1.0f };
        float Metallic = 0.0f;

        float Roughness = 0.0f;
        float AO = 1.0f;
        int AlbedoTexToggle = 0;
        int MetallicTexToggle = 0;

        int AOTexToggle = 0;
        int RoughnessTexToggle = 0;
        int NormalTexToggle = 0;
        float __Padding0;
    };

    enum class TextureMapType
    {
        ALBEDO = 1,
        METALLIC,
        NORMAL,
        ROUGHNESS,
        AO
    };

    class Material : public IElectroRef
    {
    public:
        Material() = default;
        Material(const Ref<Shader>& shader);
        ~Material() = default;

        void Bind(Uint index);

        template<typename T>
        void Set(const String& name, T& resource, int xtraData)
        {
            for (ShaderResource& res : mReflectionData.GetResources())
            {
                if (res.Name == name)
                {
                    if (std::is_same_v<T, Ref<Texture2D>>)
                    {
                        TextureMapType temp = (TextureMapType)xtraData;
                        switch (temp)
                        {
                            case TextureMapType::ALBEDO:
                                mAlbedoMap.Data1 = resource;
                                mAlbedoMap.Data2 = res.Binding; break;
                            case TextureMapType::METALLIC:
                                mMetallicMap.Data1 = resource;
                                mMetallicMap.Data2 = res.Binding; break;
                            case TextureMapType::NORMAL:
                                mNormalMap.Data1 = resource;
                                mNormalMap.Data2 = res.Binding; break;
                            case TextureMapType::ROUGHNESS:
                                mRoughnessMap.Data1 = resource;
                                mRoughnessMap.Data2 = res.Binding; break;
                            case TextureMapType::AO:
                                mAOMap.Data1 = resource;
                                mAOMap.Data2 = res.Binding; break;
                        }
                    }
                    if (std::is_same_v<T, glm::vec3>)
                    {
                        //TODO
                    }
                }
            }
        }

        Ref<Shader>& GetShader() { return mShader; }
        MaterialCbuffer& GetCBufferData() { return mCBufferData; }

        static Ref<Material> Material::Create(const Ref<Shader>& shader);
    public:
        //Paired as <Resource - BinsSlot>
        Pair<Ref<Texture2D>, Uint> mAlbedoMap;
        Pair<Ref<Texture2D>, Uint> mNormalMap;
        Pair<Ref<Texture2D>, Uint> mMetallicMap;
        Pair<Ref<Texture2D>, Uint> mRoughnessMap;
        Pair<Ref<Texture2D>, Uint> mAOMap;

    private:
        Ref<Shader> mShader;
        ShaderReflectionData mReflectionData;
        MaterialCbuffer mCBufferData;
        Ref<ConstantBuffer> mCBuffer;
    private:
        friend class MaterialPanel;
    };
}
