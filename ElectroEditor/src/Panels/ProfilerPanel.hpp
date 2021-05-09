//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"

namespace Electro
{
    class Entity;
    class ProfilerPanel
    {
    public:
        void OnImGuiRender(bool* show);

    private:
        float mTime = 0.0f;
        bool mVSync;
        float mFPSValues[50];
        Vector<float> mFrameTimes;
    };
}
