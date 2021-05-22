//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "AssetBase.hpp"
#include "AssetSerializer.hpp"
#include "Core/Ref.hpp"
#include "Core/FileSystem.hpp"
#include "Renderer/Factory.hpp"
#include "Renderer/MaterialSystem/Material.hpp"
#include <yaml-cpp/yaml.h>

#include "AssetManager.hpp"

namespace YAML
{
    template<>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };
}

namespace Electro
{
    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v);
    void AssetSerializer::SerializeMaterial(const String& path, Ref<Material>& mat)
    {
        YAML::Emitter out;

        out << YAML::BeginMap;
        out << YAML::Key << "Albedo" << YAML::Value << mat->Get<glm::vec3>("Material.Albedo");
        out << YAML::Key << "Metallic" << YAML::Value << mat->Get<float>("Material.Metallic");
        out << YAML::Key << "Roughness" << YAML::Value << mat->Get<float>("Material.Roughness");
        out << YAML::Key << "AO" << YAML::Value << mat->Get<float>("Material.AO");

        out << YAML::Key << "AlbedoTexToggle" << YAML::Value << mat->Get<int>("Material.AlbedoTexToggle");
        out << YAML::Key << "MetallicTexToggle" << YAML::Value << mat->Get<int>("Material.MetallicTexToggle");
        out << YAML::Key << "RoughnessTexToggle" << YAML::Value << mat->Get<int>("Material.RoughnessTexToggle");
        out << YAML::Key << "NormalTexToggle" << YAML::Value << mat->Get<int>("Material.NormalTexToggle");
        out << YAML::Key << "AOTexToggle" << YAML::Value << mat->Get<int>("Material.AOTexToggle");

        out << YAML::Key << "AlbedoMap" << YAML::Value << (mat->Get("AlbedoMap") ? mat->Get("AlbedoMap")->GetPath() : "");
        out << YAML::Key << "MetallicMap" << YAML::Value << (mat->Get("MetallicMap") ? mat->Get("MetallicMap")->GetPath() : "");
        out << YAML::Key << "RoughnessMap" << YAML::Value << (mat->Get("RoughnessMap") ? mat->Get("RoughnessMap")->GetPath() : "");
        out << YAML::Key << "NormalMap" << YAML::Value << (mat->Get("NormalMap") ? mat->Get("NormalMap")->GetPath() : "");
        out << YAML::Key << "AOMap" << YAML::Value << (mat->Get("AOMap") ? mat->Get("AOMap")->GetPath() : "");

        out << YAML::EndMap;
        FileSystem::WriteFile(path, out.c_str());
    }

    void AssetSerializer::DeserializeMaterial(const String& path, Ref<Material>& mat)
    {
        if (!FileSystem::ValidatePath(path))
        {
            ELECTRO_ERROR("Failed to load file from path - '%s'", path.c_str());
            return;
        }

        YAML::Node data;
        try { data = YAML::LoadFile(path); }
        catch (const YAML::ParserException& ex) { ELECTRO_ERROR("Failed to load file '%s'\n %s", path.c_str(), ex.what()); }

        mat.Reset();
        mat = Factory::CreateMaterial(AssetManager::Get<Shader>("PBR.hlsl"), "Material", path);

        mat->Set<glm::vec3>("Material.Albedo", data["Albedo"].as<glm::vec3>());
        mat->Set<float>("Material.Metallic", data["Metallic"].as<float>());
        mat->Set<float>("Material.Roughness", data["Roughness"].as<float>());
        mat->Set<float>("Material.AO", data["AO"].as<float>());

        mat->Set<int>("Material.AlbedoTexToggle", data["AlbedoTexToggle"].as<int>());
        mat->Set<int>("Material.MetallicTexToggle", data["MetallicTexToggle"].as<int>());
        mat->Set<int>("Material.RoughnessTexToggle", data["RoughnessTexToggle"].as<int>());
        mat->Set<int>("Material.NormalTexToggle", data["NormalTexToggle"].as<int>());
        mat->Set<int>("Material.AOTexToggle", data["AOTexToggle"].as<int>());

        !data["AlbedoMap"].as<String>().empty() ?    mat->Set("AlbedoMap", Factory::CreateTexture2D(data["AlbedoMap"].as<String>())) : mat;
        !data["MetallicMap"].as<String>().empty() ?  mat->Set("MetallicMap", Factory::CreateTexture2D(data["MetallicMap"].as<String>())) : mat;
        !data["RoughnessMap"].as<String>().empty() ? mat->Set("RoughnessMap", Factory::CreateTexture2D(data["RoughnessMap"].as<String>())) : mat;
        !data["NormalMap"].as<String>().empty() ?    mat->Set("NormalMap", Factory::CreateTexture2D(data["NormalMap"].as<String>())) : mat;
        !data["AOMap"].as<String>().empty() ?        mat->Set("AOMap", Factory::CreateTexture2D(data["AOMap"].as<String>())) : mat;
    }

    void AssetSerializer::SerializePhysicsMaterial(const String& path, Ref<PhysicsMaterial>& pmat)
    {
        YAML::Emitter out;

        out << YAML::BeginMap;
        out << YAML::Key << "StaticFriction" << YAML::Value << pmat->mStaticFriction;
        out << YAML::Key << "DynamicFriction" << YAML::Value << pmat->mDynamicFriction;
        out << YAML::Key << "Bounciness" << YAML::Value << pmat->mBounciness;
        out << YAML::EndMap;

        FileSystem::WriteFile(path, out.c_str());
    }

    glm::vec3 AssetSerializer::DeserializePhysicsMaterial(const String& path)
    {
        if (!FileSystem::ValidatePath(path))
            return { 0.1, 0.1, 0.1 };

        YAML::Node data;
        try { data = YAML::LoadFile(path); }
        catch (const YAML::ParserException& ex) { ELECTRO_ERROR("Failed to load file '%s'\n %s", path.c_str(), ex.what()); }

        glm::vec3 result;
        result.x = data["StaticFriction"].as<float>();
        result.y = data["DynamicFriction"].as<float>();
        result.z = data["Bounciness"].as<float>();
        return result;
    }
}
