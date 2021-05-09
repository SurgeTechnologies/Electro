//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once

namespace Electro
{
    class Timestep
    {
    public:
        Timestep(float time = 0.0f)
            :mTime(time) {}

        operator float() const { return mTime; }

        float GetSeconds() const { return mTime; }
        float GetMilliseconds() const { return mTime * 1000.0f; }
    private:
        float mTime;
    };
}