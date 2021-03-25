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
                UI::DrawColorControl3("Color", material->mColor, 150.0f);
                UI::DrawFloatControl("Shininess", &material->mShininess, 150.0f);
                UI::DrawBoolControl("Use Diffuse Texture", &material->mAlbedoTexToggle, 150.0f);
                if (UI::DrawBoolControl("Flip Texture", &material->mFlipped, 150.0f))
                    material->FlipTextures(material->mFlipped);
            }
        }
        ImGui::End();
    }
}
