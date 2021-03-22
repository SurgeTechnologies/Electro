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

        Ref<Pipeline> GetPipeline() { return mPipeline; }
        Ref<VertexBuffer> GetVertexBuffer() { return mVertexBuffer; }
        Ref<IndexBuffer> GetIndexBuffer() { return mIndexBuffer; }

        Vector<Submesh>& GetSubmeshes() { return mSubmeshes; }
        const Vector<Submesh>& GetSubmeshes() const { return mSubmeshes; }

        Ref<Material>& GetMaterial() { return mMaterial; }
        const Vector<Vertex>& GetVertices() const { return mVertices; }
        const Vector<Index>& GetIndices() const { return mIndices; }

        Ref<Shader> GetShader() { return mShader; }
        const String& GetFilePath() const { return mFilePath; }
    private:
        void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), Uint level = 0);

    private:
        Vector<Submesh> mSubmeshes;

        Ref<Pipeline> mPipeline;
        Ref<VertexBuffer> mVertexBuffer;
        Ref<IndexBuffer> mIndexBuffer;

        Vector<Vertex> mVertices;
        Vector<Index> mIndices;

        Ref<Shader> mShader;
        Ref<Material> mMaterial;

        String mFilePath;
    };
}