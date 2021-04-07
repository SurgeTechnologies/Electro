//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroUIUtils.hpp"
#include "Renderer/ElectroRendererAPISwitch.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <FontAwesome.hpp>

namespace Electro::UI
{
    ImVec4 StandardColor = ImVec4(0.0980f, 0.46667f, 0.890196f, 1.0f);
    bool DrawScriptText(const char* label, String& value, float columnWidth, bool foundScript)
    {
        bool modified = false;
        ImGui::PushID(label);

        if (!foundScript && value != "ElectroNull")
            ImGui::TextColored({ 0.9f, 0.1f, 0.1f, 1.0f }, ICON_ELECTRO_TIMES" Not Connected with ScriptEngine");
        if (value == "ElectroNull")
            ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, ICON_ELECTRO_MINUS_SQUARE" ElectroNull is used");
        if (foundScript && value != "ElectroNull")
            ImGui::TextColored({ 0.1f, 0.9f, 0.1f, 1.0f }, ICON_ELECTRO_CHECK" Connected with ScriptEngine");

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        if (!foundScript && value != "ElectroNull")
            ImGui::PushStyleColor(ImGuiCol_Text, { 0.9f, 0.1f, 0.1f, 1.0f });
        if (value == "ElectroNull")
            ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 0.0f, 1.0f });
        if (foundScript && value != "ElectroNull")
            ImGui::PushStyleColor(ImGuiCol_Text, { 0.1f, 0.9f, 0.1f, 1.0f });

        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strcpy_s(buffer, sizeof(buffer), value.c_str());

        if (ImGui::InputText("##value", buffer, sizeof(buffer)))
        {
            value = buffer;
            modified = true;
        }

        ImGui::PopStyleColor();
        ImGui::Columns(1);
        ImGui::PopID();

        return modified;
    }

    bool Text(const char* label, const char* value, float columnWidth)
    {
        bool modified = false;

        ImGui::PushID(label);

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strcpy_s(buffer, sizeof(buffer), value);

        if (ImGui::InputText("##value", buffer, sizeof(buffer)))
        {
            value = buffer;
            modified = true;
        }

        ImGui::Columns(1);
        ImGui::PopID();
        return modified;
    }

    bool TextWithoutLabel(String* source)
    {
        bool modified = false;
        ImGui::PushItemWidth(-1);
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strcpy_s(buffer, sizeof(buffer), source->c_str());

        if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
        {
            *source = String(buffer);
        }
        ImGui::PopItemWidth();
        return modified;
    }

    bool Checkbox(const char* label, bool* boolean, float columnWidth)
    {
        bool modified = false;
        ImGui::PushID(label);
        ImGui::Columns(2);

        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();

        if (ImGui::Checkbox("##value", boolean))
            modified = true;

        ImGui::Columns(1);
        ImGui::PopID();
        return modified;
    }

    bool Int(const char* label, int* value, float columnWidth)
    {
        bool modified = false;
        ImGui::PushID(label);

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();

        if (ImGui::DragInt("##value", value, 0.1f))
            modified = true;

        ImGui::Columns(1);
        ImGui::PopID();
        return modified;
    }

    bool Float(const char* label, float* value, float columnWidth)
    {
        bool modified = false;
        ImGui::PushID(label);

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();

        if (ImGui::DragFloat("##value", value, 0.1f))
            modified = true;

        ImGui::Columns(1);
        ImGui::PopID();
        return modified;
    }

    bool Float2(const char* label, glm::vec2& value, float resetValue, float columnWidth)
    {
        bool modified = false;
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];
        ImGui::PushID(label);

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 0.0f });
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
            value.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &value.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
            value.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &value.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::Columns(1);
        ImGui::PopID();
        return modified;
    }

    bool Float3(const char* label, glm::vec3& values, float resetValue, float columnWidth)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];
        ImGui::PushID(label);

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 0.0f });
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar(2);
        ImGui::Columns(1);
        ImGui::PopID();
        return true;
    }

    bool Float4(const char* label, glm::vec4& value, float resetValue, float columnWidth)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];
        ImGui::PushID(label);

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 0.0f });
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
            value.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &value.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
            value.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &value.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
            value.z = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &value.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("W", buttonSize))
            value.z = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##W", &value.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar(2);
        ImGui::Columns(1);
        ImGui::PopID();
        return true;
    }

    bool Color4(const char* label, glm::vec4& value, float columnWidth)
    {
        bool modified = false;
        ImGui::PushID(label);

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();

        ImGui::PushItemWidth(-std::numeric_limits<float>::min());
        if (ImGui::ColorEdit4("##value", glm::value_ptr(value)))
            modified = true;

        ImGui::Columns(1);
        ImGui::PopID();
        return modified;
    }

    bool Color3(const char* label, glm::vec3& value, float columnWidth)
    {
        bool modified = false;
        ImGui::PushID(label);

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();

        ImGui::PushItemWidth(-std::numeric_limits<float>::min());
        if (ImGui::ColorEdit3("##value", glm::value_ptr(value)))
            modified = true;

        ImGui::Columns(1);
        ImGui::PopID();
        return modified;
    }

    bool ToggleButton(const char* label, bool* boolToModify)
    {
        bool pressed = false;
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        float height = ImGui::GetFrameHeight();
        float width = height * 1.55f;
        float radius = height * 0.50f;

        if (ImGui::InvisibleButton(label, ImVec2(width, height)))
        {
            *boolToModify = !*boolToModify;
            pressed = true;
        }

        ImU32 colBG;
        if (ImGui::IsItemHovered())
            colBG = *boolToModify ? IM_COL32(145 + 20, 211, 68 + 20, 255) : IM_COL32(218 - 20, 218 - 20, 218 - 20, 255);
        else
            colBG = *boolToModify ? IM_COL32(145, 211, 68, 255) : IM_COL32(218, 218, 218, 255);

        drawList->AddRectFilled(p, ImVec2(p.x + width, p.y + height), colBG, height * 0.5f);
        drawList->AddCircleFilled(ImVec2(*boolToModify ? (p.x + width - radius) : (p.x + radius), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
        return pressed;
    }

    bool DrawImage(const RendererID imageID, const glm::vec2& viewportDimensions)
    {
#ifdef RENDERER_API_DX11
        ImGui::Image(imageID, ImVec2{ viewportDimensions.x, viewportDimensions.y });
#elif defined RENDERER_API_OPENGL
        ImGui::Image(imageID, ImVec2{ viewportDimensions.x, viewportDimensions.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
#endif
        return true;
    }

    bool DrawImageButton(const RendererID imageID, glm::vec2 buttonSize)
    {
#ifdef RENDERER_API_DX11
        return ImGui::ImageButton(imageID, { buttonSize.x, buttonSize.y });
#elif defined RENDERER_API_OPENGL
        return ImGui::ImageButton(imageID, { buttonSize.x, buttonSize.y }, { 0, 1 }, { 1, 0 });
#endif
    }

    void BeginViewport(const char* name)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin(name);
    }

    void EndViewport()
    {
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void BeginDockspace()
    {
        static bool dockspaceOpen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar(3);

        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 270.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }
        style.WindowMinSize.x = minWinSizeX;
    }

    void EndDockspace() { ImGui::End(); }
    ImVec4 GetStandardColor() { return StandardColor; }

    bool DrawToolTip(char* label)
    {
        bool hovered = false;
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
            ImGui::TextUnformatted(label);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
            hovered = true;
        }
        return hovered;
    }

    bool DrawColorButton(const char* label, const ImVec4& color)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        auto result = ImGui::Button(label);
        ImGui::PopStyleColor();
        return result;
    }

    bool BeginTreeNode(const char* name, bool defaultOpen)
    {
        ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
        if (defaultOpen)
            treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

        return ImGui::TreeNodeEx(name, treeNodeFlags);
    }

    void EndTreeNode()
    {
        ImGui::TreePop();
    }

    bool Dropdown(const char* label, const char** options, int32_t optionCount, int32_t* selected)
    {
        const char* current = options[*selected];
        ImGui::TextUnformatted(label);
        ImGui::SameLine();
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        bool modified = false;

        String id = "##" + String(label);
        if (ImGui::BeginCombo(id.c_str(), current))
        {
            for (int i = 0; i < optionCount; i++)
            {
                bool isSelected = (current == options[i]);
                if (ImGui::Selectable(options[i], isSelected))
                {
                    current = options[i];
                    *selected = i;
                    modified = true;
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();
        ImGui::NextColumn();
        return modified;
    }

    bool Slider(const char* label, int& value, int min, int max)
    {
        bool modified = false;
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f });

        String id = "##" + String(label);
        if (ImGui::SliderInt(id.c_str(), &value, min, max))
            modified = true;

        ImGui::PopStyleColor();
        ImGui::NextColumn();
        return modified;
    }
}
