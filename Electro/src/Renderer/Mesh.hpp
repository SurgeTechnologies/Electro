//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Asset/AssetBase.hpp"
#include "Math/BoundingBox.hpp"
#include "Renderer/Interface/Texture.hpp"
#include "Renderer/Interface/Shader.hpp"
#include "Renderer/Interface/Pipeline.hpp"
#include "Renderer/Interface/VertexBuffer.hpp"
#include "Renderer/Interface/IndexBuffer.hpp"
#include "Renderer/Interface/ConstantBuffer.hpp"
#include "Renderer/Material.hpp"
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
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
        glm::vec2 TexCoord;
    };

    struct Submesh
    {
        Uint BaseVertex;
        Uint BaseIndex;
        Uint MaterialIndex;

        Uint IndexCount;
        Uint VertexCount;
        BoundingBox BoundingBox;

        glm::mat4 Transform;
        glm::mat4 LocalTransform;
        String NodeName, MeshName;
    };

    struct Index { Uint V1, V2, V3; };

    class Mesh : public IElectroRef
    {
    public:
        Mesh(const String& filepath);
        Mesh(const Vector<Vertex>& vertices, const Vector<Index>& indices, const glm::mat4& transform);

        // Returns the path from which the Mesh was loaded
        const String& GetPath() const { return mPath; }

        // Returns the pipeline object
        const Ref<Pipeline>& GetPipeline() const { return mPipeline; }

        //Returns the vertex buffer of the mesh
        const Ref<VertexBuffer>& GetVertexBuffer() const { return mVertexBuffer; }

        // Returns the index buffer of the mesh
        const Ref<IndexBuffer>& GetIndexBuffer() const { return mIndexBuffer; }

        // Returns the submeshes of the mesh/model
        const Vector<Submesh>& GetSubmeshes() const { return mSubmeshes; }

        // Returns the materials used for the mesh
        const Vector<Ref<Material>>& GetMaterials() const { return mMaterials; }
        Vector<Ref<Material>>& GetMaterials() { return mMaterials; }

        // Gets the vertices(Raw Data) of the mesh
        const Vector<Vertex>& GetVertices() const { return mVertices; }

        // Gets the indices(Raw Data) of the mesh
        const Vector<Index>& GetIndices() const { return mIndices; }

        // Returns the Shader used by the Mesh
        const Ref<Shader>& GetShader() const { return mShader; }

        static Ref<Mesh> Create(const String& path);
    private:
        void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), Uint level = 0);
        void LoadTexture(aiMaterial* aiMaterial, Ref<Material>& material, const String& materialName, const String& toggle, aiTextureType texType) const;
        void SetValues(aiMaterial* aiMaterial, Ref<Material>& material) const;
    private:
        String mPath;

        Vector<Submesh> mSubmeshes;

        Ref<Pipeline> mPipeline;
        Ref<VertexBuffer> mVertexBuffer;
        Ref<IndexBuffer> mIndexBuffer;
        Ref<Shader> mShader;

        Vector<Vertex> mVertices;
        Vector<Index> mIndices;

        Vector<Uint> mOptimizedIndices;
        Vector<Ref<Material>> mMaterials;
    };
}
