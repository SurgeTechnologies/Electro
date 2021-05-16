//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "CodeEditorPanel.hpp"
#include "Core/System/OS.hpp"
#include "Core/FileSystem.hpp"
#include "Core/Input.hpp"
#include "UIUtils/UIUtils.hpp"
#include "UIMacros.hpp"

namespace Electro
{
    void CodeEditorPanel::Init()
    {
        ImGuiIO& io = ImGui::GetIO();
        mFont = io.Fonts->AddFontFromFileTTF("Electro/assets/fonts/JetbrainsMono/JetBrainsMono-Regular.ttf", 15.0f);
        mCodeEditor.SetShowWhitespaces(true);
    }

    void CodeEditorPanel::OnImGuiRender(bool* show)
    {
        ImGui::Begin(CODE_EDITOR_TITLE, show);
        if (ImGui::Button("Save"))
        {
            if (!mCurrentPath.empty())
                FileSystem::WriteFile(mCurrentPath, mCodeEditor.GetText());
            else
            {
                std::optional<String>& path = OS::SaveFile("(Any)");
                if (path)
                    FileSystem::WriteFile(*path, mCodeEditor.GetText());
            }
        }

        ImGui::PushFont(mFont);
        mCodeEditor.Render(CODE_EDITOR_TITLE);
        const ImGuiPayload* dropData = UI::DragAndDropTarget(READABLE_FILE_DND_ID);
        if (dropData)
            LoadFile(*(String*)dropData->Data);

        ImGui::PopFont();

        ImGui::End();
    }

    void CodeEditorPanel::LoadFile(const String& path)
    {
        mCurrentPath.clear();
        mUnsavedText.clear();

        mCurrentPath = path;
        String& extension = FileSystem::GetExtension(path);

        TextEditor::LanguageDefinition lang;
        if (extension == ".cs")
            lang = TextEditor::LanguageDefinition::CPlusPlus();
        else if (extension == ".cpp" || extension == ".cxx" || extension == ".hpp" || extension == ".h" || extension == ".hxx")
            lang = TextEditor::LanguageDefinition::CPlusPlus();
        else if (extension == ".c")
            lang = TextEditor::LanguageDefinition::C();
        else if (extension == ".hlsl")
            lang = TextEditor::LanguageDefinition::HLSL();
        else if (extension == ".glsl")
            lang = TextEditor::LanguageDefinition::GLSL();
        else if (extension == ".as")
            lang = TextEditor::LanguageDefinition::AngelScript();

        mCodeEditor.SetLanguageDefinition(lang);
        String source = FileSystem::ReadFile(path);
        mUnsavedText = source;
        mCodeEditor.SetText(source);
    }

    void CodeEditorPanel::SetText(const String& string)
    {
        mCodeEditor.SetText(string);
    }

    const String& CodeEditorPanel::GetText() const
    {
        return mCodeEditor.GetText();
    }
}
