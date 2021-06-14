//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <chrono>

namespace Electro
{
    class Timer
    {
    public:
        Timer(bool resetOnStart = true)
        {
            if (resetOnStart)
                Reset();
        }

        void Timer::Reset()
        {
            mStart = std::chrono::high_resolution_clock::now();
        }

        float Timer::Elapsed()
        {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - mStart).count() * 0.001f * 0.001f * 0.001f;
        }

        float Timer::ElapsedMillis()
        {
            return Elapsed() * 1000.0f;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> mStart;
    };
}
