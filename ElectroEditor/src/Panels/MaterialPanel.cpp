//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "MaterialPanel.hpp"
#include "Core/System/OS.hpp"
#include "Renderer/Renderer.hpp"
#include "UIUtils/UIUtils.hpp"
#include "AssetsPanel.hpp"
#include "UIMacros.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace Electro
{
    static RendererID sPrototypeTextureID = nullptr;
    const char* imageForamatStrings[] = { ".png", ".jpg", ".tga", ".bmp", ".psd", ".hdr", ".pic", "*.gif" };

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
                    std::optional<String> filename = OS::OpenFile(TextureExtensionToString(material->GetSelectedTexExtension()).c_str());
                    if (filename)
                    {
                        material->Set(label, Factory::CreateTexture2D(*filename));
                        if (material->Get(label))
                        {
                            toggle = true;
                            material->Serialize();
                        }
                    }
                }
            }
            const ImGuiPayload* dropData = UI::DragAndDropTarget(TEXTURE_DND_ID);
            if (dropData)
            {
                material->Set(label, Factory::CreateTexture2D(*static_cast<String*>(dropData->Data)));
                if (material->Get(label))
                    toggle = true;
                material->Serialize();
            }
            ImGui::NextColumn();
            if (ImGui::Checkbox("##UseMap", &useAlbedoMap))
            {
                toggle = useAlbedoMap;
                material->Serialize();
            }
            UI::ToolTip("Use");
            ImGui::SameLine();

            Ref<Texture2D>& tex = material->Get(label);
            if (ImGui::Button("Preview") && tex)
            {
                GetTexturePreviewtorage() = tex;
                ImGui::SetWindowFocus(TEXTURE_PREVIEW_TITLE);
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                tex.Reset();
                toggle = false;
                material->Serialize();
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
                if (ImGui::CollapsingHeader("Materials"))
                {
                    for (Uint i = 0; i < materials.size(); i++)
                    {
                        Ref<Material>& materialInstance = materials[i];
                        const ImGuiTreeNodeFlags nodeFlags = (selectedMaterialIndex == i ? ImGuiTreeNodeFlags_Selected : 0);
                        const bool opened = ImGui::TreeNodeEx((&materialInstance), nodeFlags, materialInstance->GetName().c_str());
                        const ImGuiPayload* dropData = UI::DragAndDropTarget(MATERIAL_DND_ID);
                        if (dropData)
                        {
                            //Create a material from dropped data + deserialize(read the data) it
                            materials[selectedMaterialIndex].Reset();
                            const Ref<Shader>& shader = AssetManager::Get<Shader>("PBR.hlsl");
                            materials[selectedMaterialIndex] = Factory::CreateMaterial(shader, "Material", *static_cast<String*>(dropData->Data));
                            materials[selectedMaterialIndex]->Deserialize();
                        }

                        if (ImGui::IsItemClicked())
                            selectedMaterialIndex = i;
                        if (opened)
                        {
                            if (ImGui::Button("Remove"))
                            {
                                //Cleanup the previous material if necessary
                                if (materials[selectedMaterialIndex])
                                    materials[selectedMaterialIndex].Reset();

                                materials[selectedMaterialIndex] = Ref<Material>::Create(AssetManager::Get<Shader>("PBR.hlsl"), "Material", DEFAULT_MATERIAL_NAME ".emat");
                            }
                            ImGui::TreePop();
                        }
                    }
                }

                // Selected material
                if (selectedMaterialIndex < materials.size())
                {
                    Ref<Material>& material = materials[selectedMaterialIndex];
                    ImGui::Text("Material: %s", material->GetName().c_str());
                    ImGui::SameLine();
                    UI::Dropdown(imageForamatStrings, 8, reinterpret_cast<int32_t*>(&material->mTextureExtension));
                    ImGui::Separator();

                    DrawMaterialProperty("AlbedoMap", material, material->Get<int>("Material.AlbedoTexToggle"), [&]()
                    {
                        glm::vec3 color = material->Get<glm::vec3>("Material.Albedo");
                        if (ImGui::ColorEdit3("##Color", glm::value_ptr(color)))
                            material->Set<glm::vec3>("Material.Albedo", color);

                        ImGui::SameLine();
                        if (ImGui::Button("Reset##Color"))
                            material->Set<glm::vec3>("Material.Albedo", { 1.0f, 1.0f, 1.0f });
                    });

                    DrawMaterialProperty("MetallicMap", material, material->Get<int>("Material.MetallicTexToggle"), [&]()
                    {
                        ImGui::PushItemWidth(-1);

                        float metallic = material->Get<float>("Material.Metallic");
                        if (ImGui::SliderFloat("##MetalnessData", &metallic, 0.0f, 1.0f))
                            material->Set<float>("Material.Metallic", metallic);

                        ImGui::PopItemWidth();
                    });

                    DrawMaterialProperty("RoughnessMap", material, material->Get<int>("Material.RoughnessTexToggle"), [&]()
                    {
                        ImGui::PushItemWidth(-1);

                        float roughness = material->Get<float>("Material.Roughness");
                        if (ImGui::SliderFloat("##RoughnessData", &roughness, 0.0f, 1.0f))
                            material->Set<float>("Material.Roughness", roughness);

                        ImGui::PopItemWidth();
                    });

                    DrawMaterialProperty("NormalMap", material, material->Get<int>("Material.NormalTexToggle"), [&]() {});

                    DrawMaterialProperty("AOMap", material, material->Get<int>("Material.AOTexToggle"), [&]()
                    {
                        float ao = material->Get<float>("Material.AO");
                        if (ImGui::DragFloat("##AOData", &ao))
                            material->Set<float>("Material.AO", ao);

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
