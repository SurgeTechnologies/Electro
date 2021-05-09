//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "MaterialPanel.hpp"
#include "UIUtils/UIUtils.hpp"
#include "AssetsPanel.hpp"
#include "UIMacros.hpp"
#include "Core/Timer.hpp"
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
        sPrototypeTextureID = AssetManager::Get<Texture2D>("Prototype.png")->GetRendererID();
    }

    void MaterialPanel::OnImGuiRender(bool* show, Entity& selectedEntity)
    {
        ImGui::Begin(MATERIAL_INSPECTOR_TITLE, show);

        if (selectedEntity && selectedEntity.HasComponent<MeshComponent>())
        {
            auto& mesh = selectedEntity.GetComponent<MeshComponent>().Mesh;
            if (mesh)
            {
                Vector<Ref<Material>>& materials = mesh->GetMaterials();
                static Uint selectedMaterialIndex = 0;
                for (uint32_t i = 0; i < materials.size(); i++)
                {
                    auto& materialInstance = materials[i];

                    ImGuiTreeNodeFlags node_flags = (selectedMaterialIndex == i ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf;
                    bool opened = ImGui::TreeNodeEx((void*)(&materialInstance), node_flags, materialInstance->GetName().c_str());
                    if (ImGui::IsItemClicked())
                    {
                        selectedMaterialIndex = i;
                    }
                    if (opened)
                        ImGui::TreePop();
                }
                // Selected material
                if (selectedMaterialIndex < materials.size())
                {

                    Ref<Material>& material = materials[selectedMaterialIndex];
                    ImGui::TextColored(UI::GetStandardColorImVec4(), "Shader: %s", material->GetShader()->GetName().c_str());
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
                        ImGui::DragFloat("##MetalnessData", &material->Get<float>("Material.Metallic"), 0.001f, 0, 1.0f);
                        ImGui::PopItemWidth();
                    });

                    DrawMaterialProperty("RoughnessMap", material, material->Get<int>("Material.RoughnessTexToggle"), [&]()
                    {
                        ImGui::PushItemWidth(-1);
                        ImGui::SliderFloat("##RoughnessData", &material->Get<float>("Material.Roughness"), 0, 1);
                        ImGui::PopItemWidth();
                    });

                    DrawMaterialProperty("NormalMap", material, material->Get<int>("Material.NormalTexToggle"), [&]() {});

                    DrawMaterialProperty("AOMap", material, material->Get<int>("Material.AOTexToggle"), [&]()
                    {
                        ImGui::DragFloat("##AOData", &material->Get<float>("Material.AO"));
                        ImGui::SameLine();
                        if (ImGui::Button("Reset##AO"))
                            material->Set<float>("Material.AO", 1.0f);
                    });
                }
            }
        }
        ImGui::End();
    }
}
