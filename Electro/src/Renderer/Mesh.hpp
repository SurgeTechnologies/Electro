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
#include "Renderer/MaterialSystem/Material.hpp"
#include <glm/glm.hpp>

#define DEFAULT_MATERIAL_NAME "DefaultMaterial"

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
        Ref<ConstantBuffer> CBuffer;

        glm::mat4 Transform;
        glm::mat4 LocalTransform;
        String NodeName, MeshName;
    };

    struct Index { Uint V1, V2, V3; };

    class Mesh : public Asset
    {
    public:
        Mesh(const String& filepath);
        Mesh(const Vector<Vertex>& vertices, const Vector<Index>& indices, const glm::mat4& transform);

        //Returns the pipeline object
        Ref<Pipeline>& GetPipeline() { return mPipeline; }

        //Returns the vertex buffer of the mesh
        Ref<VertexBuffer>& GetVertexBuffer() { return mPipeline->GetSpecification().VertexBuffer; }

        //Returns the index buffer of the mesh
        Ref<IndexBuffer>& GetIndexBuffer() { return mPipeline->GetSpecification().IndexBuffer; }

        //Returns the submeshes of the mesh/model
        const Vector<Submesh>& GetSubmeshes() const { return mSubmeshes; }

        //Returns the materials used for the mesh
        const Vector<Ref<Material>>& GetMaterials() const { return mMaterials; }
        Vector<Ref<Material>>& GetMaterials() { return mMaterials; }

        //Gets the vertices(Raw Data) of the mesh
        const Vector<Vertex>& GetVertices() const { return mVertices; }

        //Gets the indices(Raw Data) of the mesh
        const Vector<Index>& GetIndices() const { return mIndices; }

        //Returns the Shader used by the Mesh
        Ref<Shader>& GetShader() { return mPipeline->GetSpecification().Shader; }

        //Returns the filepath, from which the mesh was loaded
        const String& GetFilePath() const { return mFilePath; }
    private:
        void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), Uint level = 0);
        void LoadTexture(aiMaterial* aiMaterial, Ref<Material>& material, const String& materialName, const String& toggle, aiTextureType texType) const;
        void SetValues(aiMaterial* aiMaterial, Ref<Material>& material) const;
    private:
        Vector<Submesh> mSubmeshes;
        Ref<Pipeline> mPipeline;

        Vector<Vertex> mVertices;
        Vector<Index> mIndices;

        Vector<Ref<Material>> mMaterials;
        String mFilePath;
    };
}
