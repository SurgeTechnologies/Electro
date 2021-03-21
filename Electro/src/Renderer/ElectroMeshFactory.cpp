//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroMeshFactory.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

namespace Electro
{
    Ref<Mesh> MeshFactory::CreateCube(const glm::vec3& size)
    {
        Vector<Vertex> vertices;
        vertices.resize(8);
        vertices[0].Position = { -size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f };
        vertices[1].Position = {  size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f };
        vertices[2].Position = {  size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f };
        vertices[3].Position = { -size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f };
        vertices[4].Position = { -size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f };
        vertices[5].Position = {  size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f };
        vertices[6].Position = {  size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f };
        vertices[7].Position = { -size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f };

        vertices[0].Normal = { -1.0f, -1.0f,  1.0f };
        vertices[1].Normal = {  1.0f, -1.0f,  1.0f };
        vertices[2].Normal = {  1.0f,  1.0f,  1.0f };
        vertices[3].Normal = { -1.0f,  1.0f,  1.0f };
        vertices[4].Normal = { -1.0f, -1.0f, -1.0f };
        vertices[5].Normal = {  1.0f, -1.0f, -1.0f };
        vertices[6].Normal = {  1.0f,  1.0f, -1.0f };
        vertices[7].Normal = { -1.0f,  1.0f, -1.0f };

        Vector<Index> indices;
        indices.resize(12);
        indices[0] =  { 0, 1, 2 };
        indices[1] =  { 2, 3, 0 };
        indices[2] =  { 1, 5, 6 };
        indices[3] =  { 6, 2, 1 };
        indices[4] =  { 7, 6, 5 };
        indices[5] =  { 5, 4, 7 };
        indices[6] =  { 4, 0, 3 };
        indices[7] =  { 3, 7, 4 };
        indices[8] =  { 4, 5, 1 };
        indices[9] =  { 1, 0, 4 };
        indices[10] = { 3, 2, 6 };
        indices[11] = { 6, 7, 3 };

        return Ref<Mesh>::Create(vertices, indices, glm::mat4(1.0f));
    }
}