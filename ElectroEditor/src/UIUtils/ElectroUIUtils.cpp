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
        {
            value.x = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if(ImGui::DragFloat("##X", &value.x, 0.1f, 0.0f, 0.0f, "%.2f"))
            modified = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
        {
            value.y = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if(ImGui::DragFloat("##Y", &value.y, 0.1f, 0.0f, 0.0f, "%.2f"))
            modified = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::Columns(1);
        ImGui::PopID();
        return modified;
    }

    bool Float3(const char* label, glm::vec3& values, float resetValue, float columnWidth)
    {
        bool modified = false;
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
        {
            values.x = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if(ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
            modified = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
        {
            values.y = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if(ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
            modified = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
        {
            values.z = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if(ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
            modified = true;
        ImGui::PopItemWidth();

        ImGui::PopStyleVar(2);
        ImGui::Columns(1);
        ImGui::PopID();
        return modified;
    }

    bool Float4(const char* label, glm::vec4& value, float resetValue, float columnWidth)
    {
        bool modified = false;
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
        {
            value.x = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if(ImGui::DragFloat("##X", &value.x, 0.1f, 0.0f, 0.0f, "%.2f"))
            modified = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
        {
            value.y = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if(ImGui::DragFloat("##Y", &value.y, 0.1f, 0.0f, 0.0f, "%.2f"))
            modified = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
        {
            value.z = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if(ImGui::DragFloat("##Z", &value.z, 0.1f, 0.0f, 0.0f, "%.2f"))
            modified = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.55f, 0.5f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.7f, 0.75f, 0.7f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.9f, 0.85f, 0.9f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("W", buttonSize))
        {
            value.w = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if(ImGui::DragFloat("##W", &value.w, 0.1f, 0.0f, 0.0f, "%.2f"))
            modified = true;
        ImGui::PopItemWidth();

        ImGui::PopStyleVar(2);
        ImGui::Columns(1);
        ImGui::PopID();
        return modified;
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
        ImGui::PushID(label);
        bool pressed = false;
        if (*boolToModify)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, { 0.0980f, 0.46667f, 0.890196f, 1.0f });
            if (ImGui::Button(ICON_ELECTRO_CHECK))
            {
                *boolToModify = false;
                pressed = true;
            }
        }
        else if (!*boolToModify)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.2f, 0.1f, 1.0f });
            if (ImGui::Button(ICON_ELECTRO_TIMES))
            {
                *boolToModify = true;
                pressed = true;
            }
        }
        ImGui::PopStyleColor();
        ImGui::PopID();
        return pressed;
    }

    bool Image(const RendererID imageID, const glm::vec2& viewportDimensions)
    {
#ifdef RENDERER_API_DX11
        ImGui::Image(imageID, ImVec2{ viewportDimensions.x, viewportDimensions.y });
#elif defined RENDERER_API_OPENGL
        ImGui::Image(imageID, ImVec2{ viewportDimensions.x, viewportDimensions.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
#endif
        return true;
    }

    bool ImageButton(const RendererID imageID, glm::vec2 buttonSize)
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

    bool ToolTip(char* label)
    {
        bool hovered = false;
        if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.5)
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

    bool ColorButton(const char* label, const ImVec4& color)
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

    bool SliderInt(const char* label, int& value, int min, int max, float columnWidth)
    {
        bool modified = false;
        ImGui::PushID(label);

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f });
        ImGui::PushItemWidth(-std::numeric_limits<float>::min());
        String id = "##" + String(label);
        if (ImGui::SliderInt(id.c_str(), &value, min, max))
            modified = true;

        ImGui::Columns(1);
        ImGui::PopStyleColor();
        ImGui::PopID();
        return modified;
    }

    bool SliderFloat(const char* label, float& value, int min, int max, float columnWidth)
    {
        bool modified = false;
        ImGui::PushID(label);

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::TextUnformatted(label);
        ImGui::NextColumn();
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f });
        ImGui::PushItemWidth(-std::numeric_limits<float>::min());
        String id = "##" + String(label);
        if (ImGui::SliderFloat(id.c_str(), &value, min, max))
            modified = true;

        ImGui::Columns(1);
        ImGui::PopStyleColor();
        ImGui::PopID();
        return modified;
    }

    bool ScriptText(const char* label, String& value, float columnWidth, bool foundScript)
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

    bool BufferingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = size_arg;
        size.x -= style.FramePadding.x * 2;

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        // Render
        const float circleStart = size.x * 0.7f;
        const float circleEnd = size.x;
        const float circleWidth = circleEnd - circleStart;

        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart * value, bb.Max.y), fg_col);

        const float t = g.Time;
        const float r = size.y / 2;
        const float speed = 1.5f;

        const float a = speed * 0;
        const float b = speed * 0.333f;
        const float c = speed * 0.666f;

        const float o1 = (circleWidth + r) * (t + a - speed * (int)((t + a) / speed)) / speed;
        const float o2 = (circleWidth + r) * (t + b - speed * (int)((t + b) / speed)) / speed;
        const float o3 = (circleWidth + r) * (t + c - speed * (int)((t + c) / speed)) / speed;

        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);
    }

    bool Spinner(const char* label, float radius, int thickness)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        // Render
        window->DrawList->PathClear();

        int num_segments = 30;
        int start = (int)glm::abs(ImSin(g.Time * 1.8f) * (num_segments - 5));

        const float a_min = IM_PI * 2.0f * ((float)start) / (float)num_segments;
        const float a_max = IM_PI * 2.0f * ((float)num_segments - 3) / (float)num_segments;

        const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

        for (int i = 0; i < num_segments; i++)
        {
            const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
            window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + g.Time * 8) * radius,
                centre.y + ImSin(a + g.Time * 8) * radius));
        }

        window->DrawList->PathStroke(4293097241, false, thickness);
    }

    ImVec4 GetStandardColorImVec4() { return StandardColor; }
    glm::vec4 GetStandardColorGLMVec4() { return glm::vec4(StandardColor.x, StandardColor.y, StandardColor.z, StandardColor.w); }
}
