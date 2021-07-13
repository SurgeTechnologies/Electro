//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Mesh.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <meshoptimizer.h>

namespace Electro
{
    glm::mat4 AssimpMat4ToGlmMat4(const aiMatrix4x4& matrix)
    {
        glm::mat4 result;
        result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
        result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
        result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
        result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
        return result;
    }

    static const Uint sMeshImportFlags =
        aiProcess_Triangulate           |
        aiProcess_GenNormals            |
        aiProcess_GenUVCoords           |
        aiProcess_OptimizeMeshes        |
        aiProcess_ValidateDataStructure |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace      |
        aiProcess_FlipWindingOrder;

    Mesh::Mesh(const Vector<Vertex>& vertices, const Vector<Index>& indices, const glm::mat4& transform)
        : mVertices(vertices), mIndices(indices)
    {
        Submesh submesh;
        submesh.BaseVertex = 0;
        submesh.BaseIndex = 0;
        submesh.IndexCount = static_cast<Uint>(indices.size() * 3);
        submesh.Transform = transform;
        mSubmeshes.push_back(submesh);

        mPipeline = Pipeline::Create();
        mVertexBuffer = VertexBuffer::Create(mVertices.data(), static_cast<Uint>(mVertices.size()) * sizeof(Vertex));
        mIndexBuffer  = IndexBuffer::Create(mIndices.data(), static_cast<Uint>(std::size(mIndices)) * 3);
        mShader       = Renderer::GetShader("PBR");
        mPipeline->GenerateInputLayout(mShader);
    }

    Mesh::Mesh(const String& filepath)
        : mPath(filepath)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath, sMeshImportFlags);
        if (!scene || !scene->HasMeshes())
            Log::Error("Failed to load mesh file: {0}", filepath);

        Uint vertexCount = 0;
        Uint indexCount = 0;
        mShader = Renderer::GetShader("PBR");

        mSubmeshes.reserve(scene->mNumMeshes);
        String previousSubmeshName = "";
        for (size_t m = 0; m < scene->mNumMeshes; m++)
        {
            aiMesh* mesh = scene->mMeshes[m];
            Submesh& submesh = mSubmeshes.emplace_back();
            submesh.BaseVertex = vertexCount;
            submesh.BaseIndex = indexCount;
            submesh.MaterialIndex = mesh->mMaterialIndex;
            submesh.IndexCount = mesh->mNumFaces * 3;
            submesh.VertexCount = mesh->mNumVertices;
            submesh.MeshName = mesh->mName.C_Str();

            if (!previousSubmeshName.empty())
            {
                E_ASSERT(previousSubmeshName != submesh.MeshName, "Duplicate Submesh Names are not allowed!");
                previousSubmeshName = submesh.MeshName;
            }
            else
                previousSubmeshName = submesh.MeshName;

            vertexCount += submesh.VertexCount;
            indexCount += submesh.IndexCount;

            BoundingBox& aabb = submesh.BoundingBox;
            aabb.Reset();

            E_ASSERT(mesh->HasPositions(), "Meshes require positions.");
            E_ASSERT(mesh->HasNormals(), "Meshes require normals.");

            size_t localVertexCount = 0;
            Vector<glm::vec3> localVertexPositions;
            for (size_t i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex vertex;
                vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
                vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

                localVertexCount += 3;
                localVertexPositions.emplace_back(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

                aabb.Min.x = glm::min(vertex.Position.x, aabb.Min.x);
                aabb.Min.y = glm::min(vertex.Position.y, aabb.Min.y);
                aabb.Min.z = glm::min(vertex.Position.z, aabb.Min.z);

                aabb.Max.x = glm::max(vertex.Position.x, aabb.Max.x);
                aabb.Max.y = glm::max(vertex.Position.y, aabb.Max.y);
                aabb.Max.z = glm::max(vertex.Position.z, aabb.Max.z);

                if (mesh->HasTangentsAndBitangents())
                {
                    vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
                    vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
                }

                if (mesh->HasTextureCoords(0))
                    vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
                else
                    vertex.TexCoord = { 0.0f, 0.0f };

                mVertices.push_back(vertex);
            }

            Vector<Uint> tempIndices;
            for (size_t i = 0; i < mesh->mNumFaces; i++)
            {
                E_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Mesh Must have 3 indices!");
                Index index = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
                mIndices.push_back(index);

                tempIndices.emplace_back(mesh->mFaces[i].mIndices[0]);
                tempIndices.emplace_back(mesh->mFaces[i].mIndices[1]);
                tempIndices.emplace_back(mesh->mFaces[i].mIndices[2]);
            }

            meshopt_optimizeVertexCache(tempIndices.data(), tempIndices.data(), tempIndices.size(), localVertexCount);
            meshopt_optimizeOverdraw(tempIndices.data(), tempIndices.data(), tempIndices.size(), &localVertexPositions[0].x, localVertexCount, sizeof(glm::vec3), 1.05f);

            mOptimizedIndices.insert(mOptimizedIndices.end(), tempIndices.begin(), tempIndices.end());
        }

        TraverseNodes(scene->mRootNode);

        if (scene->HasMaterials())
        {
            mMaterials.resize(scene->mNumMaterials);
            for (Uint i = 0; i < scene->mNumMaterials; i++)
            {
                aiMaterial* assimpMaterial = scene->mMaterials[i];
                Ref<Material> material = Ref<Material>::Create(mShader, "Material", assimpMaterial->GetName().C_Str());

                mMaterials[i] = material;

                SetValues(assimpMaterial, material);
                LoadTexture(assimpMaterial, material, "AlbedoMap", "Material.AlbedoTexToggle", aiTextureType_DIFFUSE);
                LoadTexture(assimpMaterial, material, "NormalMap", "Material.NormalTexToggle", aiTextureType_HEIGHT);
                LoadTexture(assimpMaterial, material, "RoughnessMap", "Material.RoughnessTexToggle", aiTextureType_SHININESS);
                LoadTexture(assimpMaterial, material, "RoughnessMap", "Material.RoughnessTexToggle", aiTextureType_SHININESS);
                LoadTexture(assimpMaterial, material, "MetallicMap", "Material.MetallicTexToggle", aiTextureType_SPECULAR);
                LoadTexture(assimpMaterial, material, "AOMap", "Material.AOTexToggle", aiTextureType_AMBIENT_OCCLUSION);
            }
        }
        else
        {
            Ref<Material> material = Ref<Material>::Create(mShader, "Material", "Electro-DefaultMaterial");
            material->Set<int>("Material.AlbedoTexToggle", 0);
            material->Set<int>("Material.NormalTexToggle", 0);
            material->Set<int>("Material.MetallicTexToggle", 0);
            material->Set<int>("Material.RoughnessTexToggle", 0);
            material->Set<int>("Material.AOTexToggle", 0);
            material->Set<glm::vec3>("Material.Albedo", { 1.0f, 1.0f, 1.0f });
            material->Set<float>("Material.Metalness", 0.0f);
            material->Set<float>("Material.Roughness", 0.8f);
            material->Set<float>("Material.AO", 1.0f);
            mMaterials.push_back(material);
        }

        mVertexBuffer = VertexBuffer::Create(mVertices.data(), static_cast<Uint>(mVertices.size()) * sizeof(Vertex));
        mIndexBuffer  = IndexBuffer::Create(mOptimizedIndices.data(), static_cast<Uint>(mOptimizedIndices.size()));
        mPipeline     = Pipeline::Create();
        mPipeline->GenerateInputLayout(mShader);
    }

    void Mesh::TraverseNodes(aiNode* node, const glm::mat4& parentTransform, Uint level)
    {
        const glm::mat4 localTransform = AssimpMat4ToGlmMat4(node->mTransformation);
        const glm::mat4 transform = parentTransform * localTransform;

        for (Uint i = 0; i < node->mNumMeshes; i++)
        {
            const Uint mesh = node->mMeshes[i];
            auto& submesh = mSubmeshes[mesh];
            submesh.NodeName = node->mName.C_Str();
            submesh.Transform = transform;
            submesh.LocalTransform = localTransform;
        }

        for (Uint i = 0; i < node->mNumChildren; i++)
            TraverseNodes(node->mChildren[i], transform, level + 1);
    }

    void Mesh::LoadTexture(aiMaterial* aiMaterial, Ref<Material>& material, const String& texName, const String& toggle, aiTextureType texType) const
    {
        aiString aiTexPath;
        if (aiMaterial->GetTexture(texType, 0, &aiTexPath) == aiReturn_SUCCESS)
        {
            String texturePath = FileSystem::GetParentPath(mPath) + "/" + String(aiTexPath.data);
            Log::Trace("{0} path: {1}", texName, texturePath);

            Texture2DSpecification spec;
            spec.Path = texturePath;
            spec.GenerateMips = true;
            Ref<Texture2D> texture = Texture2D::Create(spec);

            if (texture->Loaded())
            {
                material->Set(texName, texture);
                material->Set<int>(toggle, 1);
            }
            else
                Log::Error("Could not load texture: {0}", texturePath);
        }
        else
            Log::Trace("No {0} pre-defined for {1}", texName, material->GetName());
    }

    void Mesh::SetValues(aiMaterial* aiMaterial, Ref<Material>& material) const
    {
        //Color
        glm::vec3 albedoColor = { 1.0f, 1.0f, 1.0f, };
        aiColor3D aiColor;
        if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor) == AI_SUCCESS)
            albedoColor = { aiColor.r, aiColor.g, aiColor.b };
        material->Set("Material.Albedo", albedoColor);
        material->Set<float>("Material.Emissive", 0.0f);

        //Roughness
        float shininess;
        if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS)
            shininess = 50.0f;
        float roughness = 1.0f - glm::sqrt(shininess / 100.0f);
        material->Set<float>("Material.Roughness", roughness);

        //Metalness
        float metalness = 0.0f;
        aiMaterial->Get(AI_MATKEY_REFLECTIVITY, metalness);
        material->Set<float>("Material.Metallic", metalness);

        //AO
        material->Set<float>("Material.AO", 1.0f);
    }

    Ref<Mesh> Mesh::Create(const String& path)
    {
        return Ref<Mesh>::Create(path);
    }
}
