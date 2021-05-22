//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "MaterialPanel.hpp"
#include "Core/Timer.hpp"
#include "Core/System/OS.hpp"
#include "UIUtils/UIUtils.hpp"
#include "AssetsPanel.hpp"
#include "UIMacros.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include "Asset/AssetSerializer.hpp"

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
                    std::optional<String> filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                    if (filename)
                    {
                        material->Set(label, Factory::CreateTexture2D(*filename));
                        if (material->Get(label))
                            toggle = true;
                    }
                }
            }
            const ImGuiPayload* dropData = UI::DragAndDropTarget(TEXTURE_DND_ID);
            if (dropData)
            {
                material->Set(label, Factory::CreateTexture2D(*static_cast<String*>(dropData->Data)));
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

    void MaterialPanel::Init() const
    {
        sPrototypeTextureID = AssetManager::Get<Texture2D>("Prototype.png")->GetRendererID();
    }

    void MaterialPanel::OnImGuiRender(bool* show, Entity& selectedEntity)
    {
        ImGui::Begin(MATERIAL_INSPECTOR_TITLE, show);

        if (selectedEntity && selectedEntity.HasComponent<MeshComponent>())
        {
            Ref<Mesh>& mesh = selectedEntity.GetComponent<MeshComponent>().Mesh;
            if (mesh)
            {
                Vector<Ref<Material>>& materials = mesh->GetMaterials();
                static Uint selectedMaterialIndex = 0;
                for (Uint i = 0; i < materials.size(); i++)
                {
                    Ref<Material>& materialInstance = materials[i];
                    const ImGuiTreeNodeFlags nodeFlags = (selectedMaterialIndex == i ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf;
                    const bool opened = ImGui::TreeNodeEx((&materialInstance), nodeFlags, materialInstance->GetName().c_str());
                    const ImGuiPayload* dropData = UI::DragAndDropTarget(MATERIAL_DND_ID);
                    if (dropData)
                        AssetSerializer::DeserializeMaterial(*static_cast<String*>(dropData->Data), materials[selectedMaterialIndex]);

                    if (ImGui::IsItemClicked())
                        selectedMaterialIndex = i;
                    if (opened)
                        ImGui::TreePop();
                }

                // Selected material
                if (selectedMaterialIndex < materials.size())
                {
                    Ref<Material>& material = materials[selectedMaterialIndex];
                    if (ImGui::Button("Serialize"))
                    {
                        if(!material->mPathInDisk.empty())
                            AssetSerializer::SerializeMaterial(material->mPathInDisk, material);
                        else
                            ELECTRO_WARN("Cannot serialize ElectroDefaultMaterial! Create a material from assets panel first!");
                    }

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
                        ImGui::SliderFloat("##MetalnessData", &material->Get<float>("Material.Metallic"), 0.0f, 1.0f);
                        ImGui::PopItemWidth();
                    });

                    DrawMaterialProperty("RoughnessMap", material, material->Get<int>("Material.RoughnessTexToggle"), [&]()
                    {
                        ImGui::PushItemWidth(-1);
                        ImGui::SliderFloat("##RoughnessData", &material->Get<float>("Material.Roughness"), 0.0f, 1.0f);
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
