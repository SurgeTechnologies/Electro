//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroTexture.hpp"
#include "Renderer/ElectroShader.hpp"
#include "Renderer/ElectroPipeline.hpp"
#include "Renderer/ElectroVertexBuffer.hpp"
#include "Renderer/ElectroIndexBuffer.hpp"
#include "Renderer/ElectroConstantBuffer.hpp"
#include "Renderer/ElectroMaterial.hpp"
#include <glm/glm.hpp>

struct aiMesh;
struct aiNode;
struct aiMaterial;
enum aiTextureType;

namespace Electro
{
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;
    };

    struct Submesh
    {
        Uint BaseVertex;
        Uint BaseIndex;
        Uint MaterialIndex;
        Uint IndexCount;
        Uint VertexCount;

        Ref<ConstantBuffer> CBuffer;

        glm::mat4 Transform;
        glm::mat4 LocalTransform;
        String NodeName, MeshName;
    };

    struct Index { Uint V1, V2, V3; };

    class Mesh : public IElectroRef
    {
    public:
        Mesh(const String& filename);
        Mesh(const Vector<Vertex>& vertices, const Vector<Index>& indices, const glm::mat4& transform);

        Ref<Pipeline> GetPipeline() { return m_Pipeline; }
        Ref<VertexBuffer> GetVertexBuffer() { return m_VertexBuffer; }
        Ref<IndexBuffer> GetIndexBuffer() { return m_IndexBuffer; }

        Vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
        const Vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }

        Ref<Material>& GetMaterial() { return m_Material; }
        const Vector<Vertex>& GetVertices() const { return m_Vertices; }
        const Vector<Index>& GetIndices() const { return m_Indices; }

        Ref<Shader> GetShader() { return m_Shader; }
        const String& GetFilePath() const { return m_FilePath; }
    private:
        void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), Uint level = 0);

    private:
        Vector<Submesh> m_Submeshes;

        Ref<Pipeline> m_Pipeline;
        Ref<VertexBuffer> m_VertexBuffer;
        Ref<IndexBuffer> m_IndexBuffer;

        Vector<Vertex> m_Vertices;
        Vector<Index> m_Indices;

        Ref<Shader> m_Shader;
        Ref<Material> m_Material;

        String m_FilePath;
    };
}