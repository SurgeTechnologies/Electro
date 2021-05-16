//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Events/Event.hpp"
#include "Core/Events/KeyEvent.hpp"
#include <ImTextEditor.h>

namespace Electro
{
    class CodeEditorPanel
    {
    public:
        void Init();
        void OnImGuiRender(bool* show);
        void LoadFile(const String& path);
        void SetText(const String& string);
        const String& GetText() const;
    private:
        ImFont* mFont;
        TextEditor mCodeEditor;
        String mUnsavedText = "";
        String mCurrentPath = "";
    };
}
