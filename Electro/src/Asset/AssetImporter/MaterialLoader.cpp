//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "MaterialLoader.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Material.hpp"
#include "Utility/YamlHelpers.hpp"
#include "Asset/AssetManager.hpp"

namespace Electro
{
    bool MaterialLoader::LoadData(AssetMetadata metaData, Ref<Asset>& asset)
    {
        Ref<Material> result = Material::Create(Renderer::GetShader("PBR"), "Material", FileSystem::GetNameWithoutExtension(metaData.Path.string()));

        YAML::Node data;
        try { data = YAML::LoadFile(AssetManager::GetAbsolutePath(metaData)); }
        catch (const YAML::ParserException& ex) { Log::Error("Failed to load file '{0}'\n{0}", metaData.Path.string(), ex.what()); }
        YAML::Node materialNode = data["Material"];

        result->Set<int>("Material.AlbedoTexToggle", materialNode["UseAlbedoMap"].as<int>());
        result->Set<int>("Material.NormalTexToggle", materialNode["UseNormalMap"].as<int>());
        result->Set<int>("Material.MetallicTexToggle", materialNode["UseMetalnessMap"].as<int>());
        result->Set<int>("Material.RoughnessTexToggle", materialNode["UseRoughnessMap"].as<int>());
        result->Set<int>("Material.AOTexToggle", materialNode["UseAOMap"].as<int>());

        result->Set<float>("Material.Emissive", materialNode["Emission"].as<float>());
        result->Set<float>("Material.Metallic", materialNode["Metallic"].as<float>());
        result->Set<float>("Material.Roughness", materialNode["Roughness"].as<float>());
        result->Set<float>("Material.AO", materialNode["AO"].as<float>());
        result->Set<glm::vec3>("Material.Albedo", materialNode["AlbedoColor"].as<glm::vec3>());

        // Textures
        result->Set("AlbedoMap", AssetManager::GetAsset<Texture2D>(materialNode["AlbedoMap"].as<uint64_t>()));
        result->Set("NormalMap", AssetManager::GetAsset<Texture2D>(materialNode["NormalMap"].as<uint64_t>()));
        result->Set("MetallicMap", AssetManager::GetAsset<Texture2D>(materialNode["MetallicMap"].as<uint64_t>()));
        result->Set("RoughnessMap", AssetManager::GetAsset<Texture2D>(materialNode["RoughnessMap"].as<uint64_t>()));
        result->Set("AOMap", AssetManager::GetAsset<Texture2D>(materialNode["AOMap"].as<uint64_t>()));

        if (result)
        {
            asset = result;
            asset->SetHandle(metaData.Handle);
            asset->SetType(metaData.Type);
            asset->SetFlag(AssetFlag::VALID);
            return true;
        }

        asset->SetFlag(AssetFlag::INVALID);
        return false;
    }

    bool MaterialLoader::SaveData(AssetMetadata metaData, const Ref<Asset>& asset)
    {
        Ref<Material>& material = asset.As<Material>();

        YAML::Emitter out;
        out << YAML::BeginMap; // Material
        out << YAML::Key << "Material" << YAML::Value;
        {
            out << YAML::BeginMap;

            out << YAML::Key << "AlbedoColor" << YAML::Value << material->Get<glm::vec3>("Material.Albedo");
            out << YAML::Key << "Emission"    << YAML::Value << material->Get<float>("Material.Emissive");
            out << YAML::Key << "Metallic"   << YAML::Value << material->Get<float>("Material.Metallic");
            out << YAML::Key << "Roughness"   << YAML::Value << material->Get<float>("Material.Roughness");
            out << YAML::Key << "AO"          << YAML::Value << material->Get<float>("Material.AO");

            out << YAML::Key << "UseAlbedoMap"    << YAML::Value << material->Get<int>("Material.AlbedoTexToggle");
            out << YAML::Key << "UseNormalMap"    << YAML::Value << material->Get<int>("Material.NormalTexToggle");
            out << YAML::Key << "UseMetalnessMap" << YAML::Value << material->Get<int>("Material.MetallicTexToggle");
            out << YAML::Key << "UseRoughnessMap" << YAML::Value << material->Get<int>("Material.RoughnessTexToggle");
            out << YAML::Key << "UseAOMap"        << YAML::Value << material->Get<int>("Material.AOTexToggle");

            SerializeTexture(material, "AlbedoMap",    out);
            SerializeTexture(material, "NormalMap",    out);
            SerializeTexture(material, "MetallicMap",  out);
            SerializeTexture(material, "RoughnessMap", out);
            SerializeTexture(material, "AOMap",        out);

            out << YAML::EndMap;
        }
        out << YAML::EndMap; // Material

        if (out.good())
        {
            FileSystem::WriteFile(AssetManager::GetAbsolutePath(metaData), out.c_str());
            return true;
        }
        return false;
    }

    void MaterialLoader::SerializeTexture(Ref<Material>& mat, const char* mapName, YAML::Emitter& out)
    {
        Ref<Texture2D> map = mat->GetTexture2D(mapName);
        bool hasMap = map ? true : false;
        AssetHandle mapHandle = hasMap ? map->GetHandle() : 0;
        out << YAML::Key << mapName << YAML::Value << mapHandle;
    }
}
