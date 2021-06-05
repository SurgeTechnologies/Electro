//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <glm/glm.hpp>

namespace Electro
{
    struct BoundingBox
    {
        glm::vec3 Min;
        glm::vec3 Max;

        void Reset()
        {
            Min = { FLT_MAX, FLT_MAX, FLT_MAX };
            Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
        }
    };

    inline Vector<glm::vec3> ExpandAABB(BoundingBox bb)
    {
        Vector<glm::vec3> b(8);
        b[0] = { bb.Min.x, bb.Min.y, bb.Min.z };
        b[1] = { bb.Max.x, bb.Min.y, bb.Min.z };
        b[2] = { bb.Max.x, bb.Max.y, bb.Min.z };
        b[3] = { bb.Min.x, bb.Max.y, bb.Min.z };
        b[4] = { bb.Min.x, bb.Min.y, bb.Max.z };
        b[5] = { bb.Max.x, bb.Min.y, bb.Max.z };
        b[6] = { bb.Max.x, bb.Max.y, bb.Max.z };
        b[7] = { bb.Min.x, bb.Max.y, bb.Max.z };
        return b;
    }
}
