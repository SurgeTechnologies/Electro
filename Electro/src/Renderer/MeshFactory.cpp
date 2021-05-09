//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "MeshFactory.hpp"

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

    Ref<Mesh> MeshFactory::CreateSphere(float radius)
    {
        Vector<Vertex> vertices;
        Vector<Index> indices;

        constexpr float latitudeBands = 30;
        constexpr float longitudeBands = 30;

        for (float latitude = 0.0f; latitude <= latitudeBands; latitude++)
        {
            float theta = static_cast<float>(latitude * M_PI / latitudeBands);
            float sinTheta = glm::sin(theta);
            float cosTheta = glm::cos(theta);

            for (float longitude = 0.0f; longitude <= longitudeBands; longitude++)
            {
                float phi = static_cast<float>(longitude * 2 * M_PI / longitudeBands);
                float sinPhi = glm::sin(phi);
                float cosPhi = glm::cos(phi);

                Vertex vertex;
                vertex.Normal = { cosPhi * sinTheta, cosTheta, sinPhi * sinTheta };
                vertex.Position = { radius * vertex.Normal.x, radius * vertex.Normal.y, radius * vertex.Normal.z };
                vertices.push_back(vertex);
            }
        }

        for (Uint latitude = 0; latitude < latitudeBands; latitude++)
        {
            for (Uint longitude = 0; longitude < longitudeBands; longitude++)
            {
                Uint first = static_cast<Uint>((latitude * (longitudeBands + 1)) + longitude);
                Uint second = static_cast<Uint>(first + longitudeBands + 1);

                indices.push_back({ first, second, first + 1 });
                indices.push_back({ second, second + 1, first + 1 });
            }
        }

        return Ref<Mesh>::Create(vertices, indices, glm::mat4(1.0f));
    }

    Ref<Mesh> MeshFactory::CreateCapsule(float radius, float height)
    {
        Vector<Vertex> vertices;
        Vector<Index> indices;

        constexpr int segments = 30;
        constexpr int pointCount = segments + 1;

        float pointsX[pointCount];
        float pointsY[pointCount];
        float pointsZ[pointCount];
        float pointsR[pointCount];

        float calcH = 0.0f;
        float calcV = 0.0f;

        for (int i = 0; i < pointCount; i++)
        {
            float calcHRadians = glm::radians(calcH);
            float calcVRadians = glm::radians(calcV);

            pointsX[i] = glm::sin(calcHRadians);
            pointsZ[i] = glm::cos(calcHRadians);
            pointsY[i] = glm::cos(calcVRadians);
            pointsR[i] = glm::sin(calcVRadians);

            calcH += 360.0f / (float)segments;
            calcV += 180.0f / (float)segments;
        }

        float yOffset = (height - (radius * 2.0f)) * 0.5f;
        if (yOffset < 0.0f)
            yOffset = 0.0f;

        int top = static_cast<int>(glm::ceil(pointCount * 0.5f));

        for (int y = 0; y < top; y++)
        {
            for (int x = 0; x < pointCount; x++)
            {
                Vertex vertex;
                vertex.Position = glm::vec3(pointsX[x] * pointsR[y], pointsY[y] + yOffset, pointsZ[x] * pointsR[y]) * radius;
                vertices.push_back(vertex);
            }
        }

        int bottom = static_cast<int>(glm::floor(pointCount * 0.5f));

        for (int y = bottom; y < pointCount; y++)
        {
            for (int x = 0; x < pointCount; x++)
            {
                Vertex vertex;
                vertex.Position = glm::vec3(pointsX[x] * pointsR[y], -yOffset + pointsY[y], pointsZ[x] * pointsR[y]) * radius;
                vertices.push_back(vertex);
            }
        }

        for (int y = 0; y < segments + 1; y++)
        {
            for (int x = 0; x < segments; x++)
            {
                Index index1;
                index1.V1 = ((y + 0) * (segments + 1)) + x + 0;
                index1.V2 = ((y + 1) * (segments + 1)) + x + 0;
                index1.V3 = ((y + 1) * (segments + 1)) + x + 1;
                indices.push_back(index1);

                Index index2;
                index2.V1 = ((y + 0) * (segments + 1)) + x + 1;
                index2.V2 = ((y + 0) * (segments + 1)) + x + 0;
                index2.V3 = ((y + 1) * (segments + 1)) + x + 1;
                indices.push_back(index2);
            }
        }

        return Ref<Mesh>::Create(vertices, indices, glm::mat4(1.0f));
    }
}
