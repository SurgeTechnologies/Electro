//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroConsolePanel.hpp"
#include "UIUtils/ElectroUIUtils.hpp"

namespace Electro
{
    Console* Console::mConsole = new Console();
    Console::Console() {}
    Console::~Console()
    {
        delete mConsole;
    }

    Console* Console::Get()
    {
        return mConsole;
    }

    void Console::OnImGuiRender(bool* show)
    {
        ImGuiStyle& style = ImGui::GetStyle();

        ImGui::Begin(ICON_ELECTRO_LIST" Console", show);

        if (ImGui::Button("Clear") || mMessages.size() > 9999)
            ClearLog();
        ImGui::SameLine();
        if (ImGui::Button(ICON_ELECTRO_COGS))
            ImGui::OpenPopup("Console Settings");

        if(ImGui::BeginPopup("Console Settings"))
        {
            ImGui::MenuItem("Enable Scroll lock", nullptr, &mScrollLockEnabled);

            ImGui::PushStyleColor(ImGuiCol_Text, mTraceColor);
            ImGui::MenuItem(ICON_ELECTRO_PAPERCLIP" Enable Trace", nullptr, &mTraceEnabled);
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, mInfoColor);
            ImGui::MenuItem(ICON_ELECTRO_INFO_CIRCLE" Enable Info", nullptr, &mInfoEnabled);
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, mDebugColor);
            ImGui::MenuItem(ICON_ELECTRO_BUG" Enable Debug", nullptr, &mDebugEnabled);
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, mWarnColor);
            ImGui::MenuItem(ICON_ELECTRO_EXCLAMATION_TRIANGLE" Enable Warn", nullptr, &mWarningEnabled);
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, mErrorColor);
            ImGui::MenuItem(ICON_ELECTRO_EXCLAMATION_CIRCLE" Enable Error", nullptr, &mErrorEnabled);
            ImGui::PopStyleColor();

            ImGui::EndPopup();
        }

        ImGui::BeginChild(ICON_ELECTRO_LIST" Console", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (auto itr = mMessages.begin(); itr != mMessages.end(); ++itr)
        {
            switch (itr->first)
            {
                case Severity::Trace:
                    if (mTraceEnabled)
                        ImGui::TextColored(mTraceColor, (ICON_ELECTRO_PAPERCLIP" " + itr->second).c_str()); break;
                case Severity::Info:
                    if (mInfoEnabled)
                        ImGui::TextColored(mInfoColor, (ICON_ELECTRO_INFO_CIRCLE" " + itr->second).c_str()); break;
                case Severity::Debug:
                    if (mDebugEnabled)
                        ImGui::TextColored(mDebugColor, (ICON_ELECTRO_BUG" " + itr->second).c_str()); break;
                case Severity::Warning:
                    if (mWarningEnabled)
                        ImGui::TextColored(mWarnColor, (ICON_ELECTRO_EXCLAMATION_TRIANGLE" " + itr->second).c_str()); break;
                case Severity::Error:
                    if (mErrorEnabled)
                        ImGui::TextColored(mErrorColor, (ICON_ELECTRO_EXCLAMATION_CIRCLE" " + itr->second).c_str()); break;
                case Severity::Critical:
                    // You can't toggle off the critical errors!
                    ImGui::TextColored(mCriticalColor, (ICON_ELECTRO_EXCLAMATION_CIRCLE" " + itr->second).c_str()); break;
            }
        }

        if (mScrollLockEnabled)
            ImGui::SetScrollY(ImGui::GetScrollMaxY() + 100);

        ImGui::EndChild();
        ImGui::End();
    }

    void Console::Submit(const String& message, Severity level)
    {
        mMessages.emplace_back(std::pair<Severity, String>(level, message));
    }

    void Console::ClearLog()
    {
        mMessages.clear();
    }
}