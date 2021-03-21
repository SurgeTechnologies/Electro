//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/ElectroVault.hpp"
#include "ElectroMesh.hpp"
#include "ElectroRenderer.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>

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

    static const Uint s_MeshImportFlags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_OptimizeMeshes | aiProcess_ValidateDataStructure | aiProcess_JoinIdenticalVertices;

    Mesh::Mesh(const Vector<Vertex>& vertices, const Vector<Index>& indices, const glm::mat4& transform)
        : m_Vertices(vertices), m_Indices(indices)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: m_Shader = Vault::Get<Shader>("MeshShader.hlsl"); break;
            case RendererAPI::API::OpenGL: m_Shader = Vault::Get<Shader>("MeshShader.glsl"); break;
        }
        m_Material = Material::Create(m_Shader);

        Submesh submesh;
        submesh.BaseVertex = 0;
        submesh.BaseIndex = 0;
        submesh.IndexCount = indices.size() * 3;
        submesh.Transform = transform;
        submesh.CBuffer = ConstantBuffer::Create(m_Shader, "Mesh", nullptr, sizeof(glm::mat4), 1, ShaderDomain::VERTEX, DataUsage::DYNAMIC);

        m_Submeshes.push_back(submesh);

       VertexBufferLayout layout =
       {
           { ShaderDataType::Float3, "M_POSITION" },
           { ShaderDataType::Float3, "M_NORMAL" },
           { ShaderDataType::Float2, "M_TEXCOORD" },
       };

       m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex), layout);
       m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), std::size(m_Indices) * 3);

       PipelineSpecification spec;
       spec.Shader = m_Shader;
       spec.VertexBuffer = m_VertexBuffer;
       spec.IndexBuffer = m_IndexBuffer;
       m_Pipeline = Pipeline::Create(spec);
    }

    Mesh::Mesh(const String& filepath)
        :m_FilePath(filepath)
    {
        auto importer = CreateScope<Assimp::Importer>();
        const aiScene* scene = importer->ReadFile(filepath, s_MeshImportFlags);
        if (!scene || !scene->HasMeshes()) ELECTRO_ERROR("Failed to load mesh file: %s", filepath.c_str());

        Uint vertexCount = 0;
        Uint indexCount = 0;

        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: m_Shader = Vault::Get<Shader>("MeshShader.hlsl"); break;
            case RendererAPI::API::OpenGL: m_Shader = Vault::Get<Shader>("MeshShader.glsl"); break;
        }

        m_Material = Material::Create(m_Shader);
        m_Submeshes.reserve(scene->mNumMeshes);
        for (size_t m = 0; m < scene->mNumMeshes; m++)
        {
            aiMesh* mesh = scene->mMeshes[m];

            Submesh& submesh = m_Submeshes.emplace_back();
            submesh.BaseVertex = vertexCount;
            submesh.BaseIndex = indexCount;
            submesh.MaterialIndex = mesh->mMaterialIndex;
            submesh.IndexCount = mesh->mNumFaces * 3;
            submesh.VertexCount = mesh->mNumVertices;
            submesh.MeshName = mesh->mName.C_Str();
            submesh.CBuffer = ConstantBuffer::Create(m_Shader, "Mesh", nullptr, sizeof(glm::mat4), 1, ShaderDomain::VERTEX, DataUsage::DYNAMIC);
            vertexCount += submesh.VertexCount;
            indexCount += submesh.IndexCount;

            E_ASSERT(mesh->HasPositions(), "Meshes require positions.");
            E_ASSERT(mesh->HasNormals(), "Meshes require normals.");
            for (size_t i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex vertex;
                vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
                vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

                if (mesh->HasTextureCoords(0))
                    vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
                else
                    vertex.TexCoord = { 0.0f, 0.0f };
                m_Vertices.push_back(vertex);
            }

            for (size_t i = 0; i < mesh->mNumFaces; i++)
            {
                E_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Mesh Must have 3 indices!");
                Index index = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
                m_Indices.push_back(index);
            }
        }

        TraverseNodes(scene->mRootNode);

        if (scene->HasMaterials())
        {
            m_Material->GetTextures().resize(scene->mNumMaterials);
            for (Uint i = 0; i < scene->mNumMaterials; i++)
            {
                auto aiMaterial = scene->mMaterials[i];

                aiString aiTexPath;
                bool hasAlbedoMap = aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == aiReturn_SUCCESS;
                if (hasAlbedoMap)
                {
                    std::filesystem::path path = m_FilePath;
                    auto parentPath = path.parent_path();
                    parentPath /= std::string(aiTexPath.data);
                    std::string texturePath = parentPath.string();

                    aiColor3D aiColor = { 0.0f, 0.0f, 0.0f };
                    aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);

                    ELECTRO_ERROR("Albedo map path = %s", texturePath.c_str());
                    auto tex = Texture2D::Create(texturePath);

                    Vault::Submit<Texture2D>(tex);
                    if (tex->Loaded())
                    {
                        m_Material->SetDiffuseTexToggle(true);
                        m_Material->PushTexture(tex, i);
                    }
                    else
                    {
                        ELECTRO_ERROR("Could not load texture: %s", texturePath.c_str());
                        m_Material->SetDiffuseTexToggle(false);
                        m_Material->SetColor({ aiColor.r, aiColor.g, aiColor.b });
                    }
                }
                else
                {
                    m_Material->SetDiffuseTexToggle(false);
                    m_Material->SetColor({ 1.0f, 1.0f, 1.0f });
                }
            }
        }

        VertexBufferLayout layout =
        {
            { ShaderDataType::Float3, "M_POSITION" },
            { ShaderDataType::Float3, "M_NORMAL" },
            { ShaderDataType::Float2, "M_TEXCOORD" },
        };

        m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex), layout);
        m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), std::size(m_Indices) * 3);

        PipelineSpecification spec;
        spec.Shader = m_Shader;
        spec.VertexBuffer = m_VertexBuffer;
        spec.IndexBuffer = m_IndexBuffer;
        m_Pipeline = Pipeline::Create(spec);
    }

    void Mesh::TraverseNodes(aiNode* node, const glm::mat4& parentTransform, Uint level)
    {
        glm::mat4 localTransform = AssimpMat4ToGlmMat4(node->mTransformation);
        glm::mat4 transform = parentTransform * localTransform;

        for (Uint i = 0; i < node->mNumMeshes; i++)
        {
            Uint mesh = node->mMeshes[i];
            auto& submesh = m_Submeshes[mesh];
            submesh.NodeName = node->mName.C_Str();
            submesh.Transform = transform;
            submesh.LocalTransform = localTransform;
        }

        for (Uint i = 0; i < node->mNumChildren; i++)
            TraverseNodes(node->mChildren[i], transform, level + 1);
    }
}
