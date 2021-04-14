//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroMaterialPanel.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace Electro
{
    void MaterialPanel::OnImGuiRender(bool* show, Entity& selectedEntity)
    {
        ImGui::Begin("Material Inspector", show);

        if (selectedEntity && selectedEntity.HasComponent<MeshComponent>())
        {
            auto& mesh = selectedEntity.GetComponent<MeshComponent>().Mesh;
            if (mesh)
            {
                auto material = mesh->GetMaterial();
                ImGui::TextColored(ImVec4(0.1f, 0.9f, 0.1f, 1.0f), "Shader: %s", material->GetShader()->GetName().c_str());
                ImGui::Separator();
                UI::Color3("Albedo", material->mCBufferData.Albedo, 150.0f);
                UI::SliderFloat("Metallic", material->mCBufferData.Metallic, 0, 1, 150.0f);
                UI::SliderFloat("Roughness", material->mCBufferData.Roughness, 0, 1, 150.0f);
                UI::Float("AO", &material->mCBufferData.AO, 150.0f);
            }
        }
        ImGui::End();
    }
}
