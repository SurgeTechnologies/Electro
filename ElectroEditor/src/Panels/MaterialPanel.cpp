//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "MaterialPanel.hpp"
#include "Core/System/OS.hpp"
#include "Renderer/Renderer.hpp"
#include "Asset/AssetManager.hpp"
#include "UIUtils/UIUtils.hpp"
#include "AssetsPanel.hpp"
#include "AssetImportPopup.hpp"
#include "UIMacros.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace Electro
{
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
                Ref<Texture2D>& tex = material->GetTexture2D(label);
                UI::Image(tex ? tex->GetRendererID() : nullptr, { 50, 50 });

                const ImGuiPayload* droppedData = UI::DragAndDropTarget(TEXTURE_DND_ID);
                if (droppedData != nullptr)
                {
                    AssetDropData assetDropData = *(AssetDropData*)droppedData->Data;
                    if (assetDropData.Handle != INVALID_ASSET_HANDLE)
                        tex = AssetManager::GetAsset<Texture2D>(assetDropData.Handle);
                    else
                        AssetImportPopup::ThrowTextureImportPopup(assetDropData.Path);
                }
            }

            ImGui::NextColumn();
            func();
            if (ImGui::Checkbox("##UseMap", &useAlbedoMap))
            {
                toggle = useAlbedoMap;
            }
            UI::ToolTip("Use");
            ImGui::SameLine();

            if (ImGui::Button("Remove"))
            {
                material->RemoveTexture2D(label);
                toggle = false;
            }
            ImGui::Columns(1);
        }

        AssetImportPopup::CatchTextureImportPopup();
        ImGui::PopID();
    }

    void MaterialPanel::OnInit(void* hierarchy)
    {
        mSceneHierarchyPanel = (SceneHierarchyPanel*)hierarchy;
    }

    void MaterialPanel::OnImGuiRender(bool* show)
    {
        ImGui::Begin(MATERIAL_INSPECTOR_TITLE, show);
        Entity& selectedEntity = mSceneHierarchyPanel->GetSelectedEntity();
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
                    const ImGuiTreeNodeFlags nodeFlags = (selectedMaterialIndex == i ? ImGuiTreeNodeFlags_Selected : 0);
                    const bool opened = ImGui::TreeNodeEx((&materialInstance), nodeFlags, materialInstance->GetName().c_str());

                    if (ImGui::IsItemClicked())
                        selectedMaterialIndex = i;

                    if (opened)
                        ImGui::TreePop();
                }

                // Selected material
                if (selectedMaterialIndex < materials.size())
                {
                    Ref<Material>& material = materials[selectedMaterialIndex];
                    ImGui::Text("Selected Material: %s", material->GetName().c_str());
                    ImGui::Separator();

                    DrawMaterialProperty("AlbedoMap", material, material->Get<int>("Material.AlbedoTexToggle"), [&]()
                    {
                        glm::vec3 color = material->Get<glm::vec3>("Material.Albedo");
                        if (ImGui::ColorEdit3("##Color", glm::value_ptr(color)))
                            material->Set<glm::vec3>("Material.Albedo", color);

                        ImGui::TextUnformatted("Emissive");
                        ImGui::SameLine();

                        float emissive = material->Get<float>("Material.Emissive");
                        if (ImGui::DragFloat("##Material.Emissive", &emissive))
                        {
                            material->Set<float>("Material.Emissive", emissive);
                            material->Set<float>("Material.Roughness", 0.0f);
                            material->Set<float>("Material.Metallic", 0.0f);
                        }
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
