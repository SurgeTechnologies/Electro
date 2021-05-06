//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroMaterialPanel.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include "ElectroVaultPanel.hpp"
#include "ElectroUIMacros.hpp"
#include "Core/ElectroTimer.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace Electro
{
    static RendererID sPrototypeTextureID = nullptr;

    template<typename UIFunction>
    void DrawMaterialProperty(const char* label, Ref<Material>& material, int& toggle, UIFunction func)
    {
        ImGui::PushID(label);
        if(ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool useAlbedoMap = toggle;
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 68);
            {
                Ref<Texture2D> tex = material->Get(label);
                if (UI::ImageButton(tex ? tex->GetRendererID() : sPrototypeTextureID, { 50, 50 }))
                {
                    auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                    if (filename)
                    {
                        material->Set(label, EGenerator::CreateTexture2D(*filename));
                        if (material->Get(label))
                            toggle = true;
                    }
                }
            }
            auto dropData = UI::DragAndDropTarget(TEXTURE_DND_ID);
            if (dropData)
            {
                material->Set(label, EGenerator::CreateTexture2D(*(String*)dropData->Data));
                if (material->Get(label))
                    toggle = true;
            }
            ImGui::NextColumn();
            if (ImGui::Checkbox("##UseMap", &useAlbedoMap))
                toggle = useAlbedoMap;
            UI::ToolTip("Use");
            ImGui::SameLine();

            Ref<Texture2D>& tex = material->Get(label);
            if (ImGui::Button("Preview") && tex)
            {
                GetTexturePreviewtorage() = tex;
                ImGui::SetWindowFocus("Texture Preview");
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                tex.Reset();
                toggle = false;
            }
            func();
            ImGui::Columns(1);
        }

        ImGui::PopID();
    }

    void MaterialPanel::Init()
    {
        sPrototypeTextureID = Vault::Get<Texture2D>("Prototype.png")->GetRendererID();
    }

    void MaterialPanel::OnImGuiRender(bool* show, Entity& selectedEntity)
    {
        ImGui::Begin(MATERIAL_INSPECTOR_TITLE, show);

        if (selectedEntity && selectedEntity.HasComponent<MeshComponent>())
        {
            auto& mesh = selectedEntity.GetComponent<MeshComponent>().Mesh;
            Ref<Material> material;
            if (mesh)
            {
                material = mesh->GetMaterial();
                ImGui::TextColored(ImVec4(0.1f, 0.9f, 0.1f, 1.0f), "Shader: %s", material->GetShader()->GetName().c_str());
                ImGui::Separator();
                DrawMaterialProperty("AlbedoMap", material, material->Get<int>("Material.AlbedoTexToggle"), [&]()
                {
                    ImGui::ColorEdit3("##Color", glm::value_ptr(material->Get<glm::vec3>("Material.Albedo")));
                    ImGui::SameLine();
                    if (ImGui::Button("Reset##Color"))
                        material->Set<glm::vec3>("Material.Albedo", { 1.0f, 1.0f, 1.0f });
                });

                DrawMaterialProperty("MetallicMap", material, material->Get<int>("Material.MetallicTexToggle"), [&]()
                {
                    ImGui::PushItemWidth(-1);
                    ImGui::DragFloat("##MetalnessData", &material->Get<float>("Material.Metallic"), 0.001, 0, 1);
                    ImGui::PopItemWidth();
                });

                DrawMaterialProperty("RoughnessMap", material, material->Get<int>("Material.RoughnessTexToggle"), [&]()
                {
                    ImGui::PushItemWidth(-1);
                    ImGui::SliderFloat("##RoughnessData", &material->Get<float>("Material.Roughness"), 0, 1);
                    ImGui::PopItemWidth();
                });

                DrawMaterialProperty("NormalMap", material, material->Get<int>("Material.NormalTexToggle"), [&](){});

                DrawMaterialProperty("AOMap", material, material->Get<int>("Material.AOTexToggle"), [&]()
                {
                    ImGui::DragFloat("##AOData", &material->Get<float>("Material.AO"));
                    ImGui::SameLine();
                    if (ImGui::Button("Reset##AO"))
                        material->Set<float>("Material.AO", 1.0f);
                });
            }
        }
        ImGui::End();
    }
}
