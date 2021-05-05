//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroMaterialPanel.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include "ElectroVaultPanel.hpp"
#include "ElectroUIMacros.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace Electro
{
    static RendererID sPrototypeTextureID = nullptr;

    template<typename UIFunction>
    void DrawMaterialProperty(const char* label, Ref<Material>& material, Ref<Texture2D>& texture, int& toggle, TextureMapType mapType, UIFunction func)
    {
        ImGui::PushID(label);
        if(ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto& bufferData = material->GetCBufferData();
            bool useAlbedoMap = toggle;
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 68);
            if (UI::ImageButton(texture ? texture->GetRendererID() : sPrototypeTextureID, { 50, 50 }))
            {
                auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                if (filename)
                {
                    material->Set<Ref<Texture2D>>(label, EGenerator::CreateTexture2D(*filename), (int)mapType);
                    if (texture)
                        toggle = true;
                }
            }
            auto dropData = UI::DragAndDropTarget(TEXTURE_DND_ID);
            if (dropData)
            {
                material->Set<Ref<Texture2D>>(label, texture = EGenerator::CreateTexture2D(*(String*)dropData->Data), (int)mapType);
                if (texture)
                    toggle = true;
            }
            ImGui::NextColumn();
            if (ImGui::Checkbox("##UseMap", &useAlbedoMap))
                toggle = useAlbedoMap;
            UI::ToolTip("Use");
            ImGui::SameLine();
            if (ImGui::Button("Preview") && texture)
            {
                GetTexturePreviewtorage() = texture;
                ImGui::SetWindowFocus("Texture Preview");
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                texture.Reset();
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
                auto& cbufferData = material->GetCBufferData();
                ImGui::TextColored(ImVec4(0.1f, 0.9f, 0.1f, 1.0f), "Shader: %s", material->GetShader()->GetName().c_str());
                ImGui::Separator();
                DrawMaterialProperty("AlbedoMap", material, material->mAlbedoMap.Data1, cbufferData.AlbedoTexToggle, TextureMapType::ALBEDO, [&]()
                {
                    ImGui::ColorEdit3("##Color", glm::value_ptr(cbufferData.Albedo));
                    ImGui::SameLine();
                    if (ImGui::Button("Reset##Color"))
                        cbufferData.Albedo = { 1.0f, 1.0f, 1.0f };
                });

                DrawMaterialProperty("MetallicMap", material, material->mMetallicMap.Data1, cbufferData.MetallicTexToggle, TextureMapType::METALLIC, [&]()
                {
                    ImGui::PushItemWidth(-1);
                    ImGui::SliderFloat("##MetalnessData", &cbufferData.Metallic, 0, 1);
                    ImGui::PopItemWidth();
                });

                DrawMaterialProperty("RoughnessMap", material, material->mRoughnessMap.Data1, cbufferData.RoughnessTexToggle, TextureMapType::ROUGHNESS, [&]()
                {
                    ImGui::PushItemWidth(-1);
                    ImGui::SliderFloat("##RoughnessData", &cbufferData.Roughness, 0, 1);
                    ImGui::PopItemWidth();
                });

                DrawMaterialProperty("NormalMap", material, material->mNormalMap.Data1, cbufferData.NormalTexToggle, TextureMapType::NORMAL, [&](){});

                DrawMaterialProperty("AOMap", material, material->mAOMap.Data1, cbufferData.AOTexToggle, TextureMapType::AO, [&]()
                {
                    ImGui::DragFloat("##AOData", &cbufferData.AO);
                    ImGui::SameLine();
                    if (ImGui::Button("Reset##AO"))
                        cbufferData.AO = 1.0f;
                });
            }
        }
        ImGui::End();
    }
}
