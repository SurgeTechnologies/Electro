//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "PhysicsMaterialEditor.hpp"
#include "Asset/AssetManager.hpp"
#include <imgui.h>

namespace Electro
{
    void PhysicsMaterialEditor::Render()
    {
        if (!mCurrentPhysicsMaterial)
            return;

        const float& width = ImGui::GetWindowWidth();
        ImGui::Button(mPhysicsMaterialNameBuffer.c_str(), ImVec2(width, 0.0f));

        if (ImGui::BeginTable("##pmatEditor", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter))
        {
            // Static Friction
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Static Friction");
            ImGui::TableNextColumn();
            ImGui::PushItemWidth(-1);
            if (ImGui::DragFloat("##sFric", &mCurrentPhysicsMaterial->GetStaticFriction()))
                SerializePhysicsMaterial();
            ImGui::PopItemWidth();

            // Dynamic Friction
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Dynamic Friction");
            ImGui::TableNextColumn();
            ImGui::PushItemWidth(-1);
            if (ImGui::DragFloat("##dFric", &mCurrentPhysicsMaterial->GetDynamicFriction()))
                SerializePhysicsMaterial();
            ImGui::PopItemWidth();

            // Bounciness
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Bounciness");
            ImGui::TableNextColumn();
            ImGui::PushItemWidth(-1);
            if (ImGui::DragFloat("##bounciness", &mCurrentPhysicsMaterial->GetBounciness()))
                SerializePhysicsMaterial();
            ImGui::PopItemWidth();

            ImGui::EndTable();
        }
    }

    void PhysicsMaterialEditor::SetForEdit(Ref<Asset>& asset)
    {
        if (mCurrentPhysicsMaterial)
            mCurrentPhysicsMaterial = nullptr;

        mCurrentPhysicsMaterial = asset.As<PhysicsMaterial>().Raw();
        if (mCurrentPhysicsMaterial)
        {
            mPhysicsMaterialNameBuffer.clear();
            mPhysicsMaterialNameBuffer = AssetManager::GetMetadata(mCurrentPhysicsMaterial->GetHandle()).Path.filename().string();
        }
        else
        {
            Log::Error("Invalid Material!");
        }
    }
}