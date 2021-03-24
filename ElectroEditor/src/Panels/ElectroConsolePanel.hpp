//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#define ELECTRO_ENABLE_IMGUI_CONSOLE
#include "Core/ElectroLog.hpp"
#include <imgui.h>
#include <FontAwesome.hpp>
#include <vector>
#include <string>

namespace Electro
{
    class Console
    {
    public:
        ~Console();
        static Console* Get();
        void OnImGuiRender(bool* show);
        void Submit(const String& message, Severity level = Severity::Info);

    private:
        Console();
        void ClearLog();
    private:
        static Console* mConsole;
        Vector<std::pair<Severity, String>> mMessages{};
        bool mScrollLockEnabled = true;

        //Colors
        ImVec4 mTraceColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        ImVec4 mInfoColor = { 0.1f, 0.9f, 0.1f, 1.0f };
        ImVec4 mDebugColor = { 0.0f, 0.5f, 1.0f, 1.0f };
        ImVec4 mWarnColor = { 1.0f, 0.9f, 0.0f, 1.0f };
        ImVec4 mErrorColor = { 1.0f, 0.2f, 0.1f, 1.0f };
        ImVec4 m_CriticalColor = { 0.5f, 0.0f, 0.7f, 1.0f };
        ImVec4 mEnabledColor = { 0.1f, 0.1f, 0.1f, 1.0f };
        ImVec4 mDisabledColor = { 0.5f, 0.2f, 0.2f, 1.0f };

        //Bool switches
        bool mWarningEnabled = true;
        bool mTraceEnabled = true;
        bool mInfoEnabled = true;
        bool mDebugEnabled = true;
        bool mErrorEnabled = true;
    };
}