//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "PhysicsMaterialLoader.hpp"
#include "Physics/PhysicsMaterial.hpp"
#include "Asset/AssetManager.hpp"
#include <yaml-cpp/yaml.h>

namespace Electro
{
    bool PhysicsMaterialLoader::LoadData(AssetMetadata metaData, Ref<Asset>& asset)
    {
        Ref<PhysicsMaterial>& result = PhysicsMaterial::Create();

        YAML::Node data;
        try { data = YAML::LoadFile(AssetManager::GetAbsolutePath(metaData)); }
        catch (const YAML::ParserException& ex) { Log::Error("Failed to load file '{0}'\n{0}", metaData.Path.string(), ex.what()); }
        YAML::Node pmatNode = data["PhysicsMaterial"];

        result->SetStaticFriction(pmatNode["StaticFriction"].as<float>());
        result->SetDynamicFriction(pmatNode["DynamicFriction"].as<float>());
        result->SetBounciness(pmatNode["Bounciness"].as<float>());

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

    bool PhysicsMaterialLoader::SaveData(AssetMetadata metaData, const Ref<Asset>& asset)
    {
        const Ref<PhysicsMaterial>& pmat = asset.As<PhysicsMaterial>();

        YAML::Emitter out;
        out << YAML::BeginMap; // Material
        out << YAML::Key << "PhysicsMaterial" << YAML::Value;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "StaticFriction" << YAML::Value << pmat->GetStaticFriction();
            out << YAML::Key << "DynamicFriction" << YAML::Value << pmat->GetDynamicFriction();
            out << YAML::Key << "Bounciness" << YAML::Value << pmat->GetBounciness();
            out << YAML::EndMap;
        }
        out << YAML::EndMap; // PhysicsMaterial

        if (out.good())
        {
            FileSystem::WriteFile(AssetManager::GetAbsolutePath(metaData), out.c_str());
            return true;
        }
        return false;
    }
}