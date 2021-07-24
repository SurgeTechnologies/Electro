//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "MaterialPanel.hpp"
#include "UIUtils/UIUtils.hpp"
#include "AssetsPanel.hpp"
#include "AssetImportPopup.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace Electro
{
    template<typename UIFunction, typename SerializeFunc>
    void DrawMaterialProperty(const char* label, Material* material, int& toggle, UIFunction func, SerializeFunc serializeFunc)
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
                    {
                        // Asset is present in registry, get that!
                        tex = AssetManager::GetAsset<Texture2D>(assetDropData.Handle);
                        toggle = true;
                        serializeFunc();
                    }
                    else
                    {
                        // Handle is invalid, propt user to add texture as an asset
                        AssetImportPopup::ThrowImportPopup(AssetType::TEXTURE2D, assetDropData.Path);
                    }
                }
            }

            ImGui::NextColumn();
            func();
            if (ImGui::Checkbox("##UseMap", &useAlbedoMap))
            {
                toggle = useAlbedoMap;
                serializeFunc();
            }
            UI::ToolTip("Use");
            ImGui::SameLine();

            if (ImGui::Button("Remove"))
            {
                material->RemoveTexture2D(label);
                serializeFunc();
            }

            ImGui::Columns(1);
        }

        AssetImportPopup::CatchImportPopup(AssetType::TEXTURE2D); // Catch all thrown popups!
        ImGui::PopID();
    }

    void MaterialPanel::OnInit(void* data)
    {
        mCurrentMaterial = nullptr;
        std::memset(mMaterialNameBuffer, 0, sizeof(mMaterialNameBuffer));
    }

    void MaterialPanel::OnImGuiRender(bool* show)
    {
        ImGui::Begin(MATERIAL_INSPECTOR_TITLE, show);

        if (mMaterialNameBuffer[0] == NULL)
            ImGui::Button("##NULL", ImVec2(ImGui::GetWindowWidth() - 18.0f, 0.0f));
        else
            ImGui::Button(mMaterialNameBuffer, ImVec2(ImGui::GetWindowWidth() - 18.0f, 0.0f));

        UI::InstantToolTip("Drag and Drop 'Materials' here to edit them!");

        const ImGuiPayload* droppedData = UI::DragAndDropTarget(MATERIAL_DND_ID);
        if (droppedData)
        {
            AssetHandle handle = *(AssetHandle*)droppedData->Data;
            if (handle != INVALID_ASSET_HANDLE)
                SetMaterial(AssetManager::GetAsset<Material>(handle));
        }

        if (mCurrentMaterial)
        {
            DrawMaterialProperty("AlbedoMap", mCurrentMaterial, mCurrentMaterial->Get<int>("Material.AlbedoTexToggle"), [&]()
            {
                bool changed = false;
                glm::vec3 color = mCurrentMaterial->Get<glm::vec3>("Material.Albedo");
                if (ImGui::ColorEdit3("##Color", glm::value_ptr(color)))
                {
                    mCurrentMaterial->Set<glm::vec3>("Material.Albedo", color);
                    changed = true;
                }

                ImGui::TextUnformatted("Emissive");
                ImGui::SameLine();

                float emissive = mCurrentMaterial->Get<float>("Material.Emissive");
                if (ImGui::DragFloat("##Material.Emissive", &emissive))
                {
                    mCurrentMaterial->Set<float>("Material.Emissive", emissive);
                    changed = true;
                }

                if (changed)
                    SerializeMaterial();
            }, [&]() { SerializeMaterial(); });

            DrawMaterialProperty("MetallicMap", mCurrentMaterial, mCurrentMaterial->Get<int>("Material.MetallicTexToggle"), [&]()
            {
                ImGui::PushItemWidth(-1);

                bool changed = false;
                float metallic = mCurrentMaterial->Get<float>("Material.Metallic");
                if (ImGui::SliderFloat("##MetalnessData", &metallic, 0.0f, 1.0f))
                {
                    mCurrentMaterial->Set<float>("Material.Metallic", metallic);
                    changed = true;
                }

                if (changed)
                    SerializeMaterial();

                ImGui::PopItemWidth();
            }, [&]() { SerializeMaterial(); });

            DrawMaterialProperty("RoughnessMap", mCurrentMaterial, mCurrentMaterial->Get<int>("Material.RoughnessTexToggle"), [&]()
            {
                ImGui::PushItemWidth(-1);

                bool changed = false;
                float roughness = mCurrentMaterial->Get<float>("Material.Roughness");
                if (ImGui::SliderFloat("##RoughnessData", &roughness, 0.0f, 1.0f))
                {
                    mCurrentMaterial->Set<float>("Material.Roughness", roughness);
                    changed = true;
                }

                if (changed)
                    SerializeMaterial();

                ImGui::PopItemWidth();
            }, [&]() { SerializeMaterial(); });

            DrawMaterialProperty("NormalMap", mCurrentMaterial, mCurrentMaterial->Get<int>("Material.NormalTexToggle"), [&]() {}, [&]() { SerializeMaterial(); });

            DrawMaterialProperty("AOMap", mCurrentMaterial, mCurrentMaterial->Get<int>("Material.AOTexToggle"), [&]()
            {
                bool changed = false;
                float ao = mCurrentMaterial->Get<float>("Material.AO");
                if (ImGui::DragFloat("##AOData", &ao))
                {
                    mCurrentMaterial->Set<float>("Material.AO", ao);
                    changed = true;
                }

                ImGui::SameLine();
                if (ImGui::Button("Reset##AO"))
                {
                    mCurrentMaterial->Set<float>("Material.AO", 1.0f);
                    changed = true;
                }

                if (changed)
                    SerializeMaterial();
            }, [&]() { SerializeMaterial(); });
        }

        ImGui::End();
    }

    void MaterialPanel::SetMaterial(Ref<Material>& mat)
    {
        if (mCurrentMaterial)
            mCurrentMaterial = nullptr;

        mCurrentMaterial = mat.Raw();
        if (mCurrentMaterial)
        {
            std::memset(mMaterialNameBuffer, 0, sizeof(mMaterialNameBuffer));
            std::memcpy(mMaterialNameBuffer, mCurrentMaterial->GetName().c_str(), INPUT_BUFFER_LENGTH);
        }
        else
        {
            Log::Error("Invalid Material!");
        }
    }
}
