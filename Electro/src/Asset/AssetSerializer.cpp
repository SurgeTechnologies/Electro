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

namespace Electro
{
    void AssetSerializer::SerializeMaterial(const String& path, Ref<Material>& mat)
    {
        std::ofstream out(path, std::ios::binary);
        if (out)
        {
            out.write(static_cast<char*>(mat->mCBufferMemory.GetData()), mat->mCBufferMemory.GetSize());

            /*for (const Ref<Texture2D>& tex : mat->mTextures)
            {
                if(tex)
                    out.write(tex->GetPath().c_str(), tex->GetPath().size());
                else
                    out.write("Empty.jpg", 9);
            }*/
            out.close();
        }
    }

    void AssetSerializer::DeserializeMaterial(const String& path, Ref<Material>& mat)
    {
        std::ifstream in(path);
        if (in)
        {
            //Get the file size
            in.seekg(0, std::ios::end);
            const auto size = in.tellg();
            in.seekg(0, std::ios::beg);

            const Ref<Shader> shader = mat->GetShader();
            const String nameInShader = mat->mBufferName;
            mat.Reset();
            mat = Factory::CreateMaterial(shader, nameInShader, path);
            in.read(static_cast<char*>(mat->mCBufferMemory.GetData()), mat->mCBufferMemory.GetSize());

            /*if(size != mat->mCBufferMemory.GetSize())
            {
                //Make sure that the cursor is after the mCBufferMemory size
                in.seekg(mat->mCBufferMemory.GetSize());

                //Get the actual vector
                const Vector<char> vec = Vector<char>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

                Vector<String> paths;
                const char* ext = ".jpg";
                const size_t n = std::strlen(ext);

                for (auto first = std::begin(vec), last = std::end(vec), it = first;
                     (it = std::search(first, last, ext, ext + n)) != last;
                     first = it
                )
                {
                    std::advance(it, n);
                    paths.push_back({first, it });
                }

                for (Uint i = 0; i < paths.size(); i++)
                {
                    if(paths[i] == "Empty.jpg")
                        continue;
                    mat->mTextures[i] = Factory::CreateTexture2D(paths[i]);
                }
            }*/
            in.close();
        }
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
