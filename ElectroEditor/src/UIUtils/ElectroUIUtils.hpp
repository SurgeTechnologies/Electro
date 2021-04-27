//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <string>
#include "Scene/ElectroEntity.hpp"
#include <imgui.h>
#include <glm/glm.hpp>

namespace Electro::UI
{
    void BeginViewport(const char* name);
    void EndViewport();

    bool BeginTreeNode(const char* name, bool defaultOpen = true);
    void EndTreeNode();

    void BeginDockspace();
    void EndDockspace();

    bool ToolTip(char* label);
    bool ColorButton(const char* label, const ImVec4& color);
    bool Image(const RendererID imageID, const glm::vec2& viewportDimensions);
    bool ImageButton(const RendererID imageID, glm::vec2 buttonSize, ImVec4 buttonBGColor = { 0.0f, 0.0f, 0.0f, 0.0f });
    bool ScriptText(const char* label, String& value, float columnWidth = 100.0f, bool foundTheScript = false);
    bool Text(const char* label, const char* value, float columnWidth = 100.0f);
    bool TextWithoutLabel(String* source);
    void TextCentered(const String& text);
    bool Checkbox(const char* label, bool* boolean, float columnWidth = 100.0f);
    bool Int(const char* label, int* value, float columnWidth = 100.0f);
    bool Float(const char* label, float* value, float columnWidth = 100.0f);
    bool Float3(const char* label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
    bool Float2(const char* label, glm::vec2& value, float resetValue = 0.0f, float columnWidth = 100.0f);
    bool Float4(const char* label, glm::vec4& value, float resetValue = 0.0f, float columnWidth = 100.0f);
    bool Color4(const char* label, glm::vec4& value, float columnWidth = 100.0f);
    bool Color3(const char* label, glm::vec3& value, float columnWidth = 100.0f);
    bool Dropdown(const char* label, const char** options, int32_t optionCount, int32_t* selected);
    bool SliderInt(const char* label, int& value, int min, int max, float columnWidth = 100.0f);
    bool SliderFloat(const char* label, float& value, int min, int max, float columnWidth = 100.0f);
    bool ToggleButton(const char* label, bool* boolToModify);
    bool BufferingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col);
    bool Spinner(const char* label, float radius, int thickness);
    void DragAndDropSource(const char* uniqueID, void* data, int dataSize, const char* tip = "...");
    const ImGuiPayload* DragAndDropTarget(const char* uniqueID);
    ImVec4 GetStandardColorImVec4();
    glm::vec4 GetStandardColorGLMVec4();
}

