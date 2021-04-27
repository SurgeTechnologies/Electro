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
    void DrawMaterialProperty(const char* label, Ref<Material>& material, Ref<Texture2D>& texToReplace, int& toggle, UIFunction func)
    {
        ImGui::PushID(label);
        if(ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto& bufferData = material->GetCBufferData();
            bool useAlbedoMap = toggle;
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 68);
            if (UI::ImageButton(texToReplace ? texToReplace->GetRendererID() : sPrototypeTextureID, { 50, 50 }))
            {
                auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                if (filename)
                {
                    texToReplace = EDevice::CreateTexture2D(*filename);
                    if (texToReplace)
                        toggle = true;
                }
            }
            auto dropData = UI::DragAndDropTarget(TEXTURE_DND_ID);
            if (dropData)
            {
                texToReplace = EDevice::CreateTexture2D(*(String*)dropData->Data);
                if (texToReplace)
                    toggle = true;
            }
            ImGui::NextColumn();
            if (ImGui::Checkbox("##UseMap", &useAlbedoMap))
                toggle = useAlbedoMap;
            UI::ToolTip("Use");
            ImGui::SameLine();
            if (ImGui::Button("Flip") && texToReplace)
                texToReplace->ReloadFlipped();
            ImGui::SameLine();
            if (ImGui::Button("Preview") && texToReplace)
            {
                GetTexturePreviewtorage() = texToReplace;
                ImGui::SetWindowFocus("Texture Preview");
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                texToReplace.Reset();
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
        ImGui::Begin("Material Inspector", show);

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
                DrawMaterialProperty("Albedo", material, material->mAlbedoMap, cbufferData.AlbedoTexToggle, [&]()
                {
                    ImGui::ColorEdit3("##Color", glm::value_ptr(cbufferData.Albedo));
                    ImGui::SameLine();
                    if (ImGui::Button("Reset##Color"))
                        cbufferData.Albedo = { 1.0f, 1.0f, 1.0f };
                });

                DrawMaterialProperty("Metalness", material, material->mMetallicMap, cbufferData.MetallicTexToggle, [&]()
                {
                    ImGui::PushItemWidth(-1);
                    ImGui::SliderFloat("##MetalnessData", &cbufferData.Metallic, 0, 1);
                    ImGui::PopItemWidth();
                });

                DrawMaterialProperty("Roughness", material, material->mRoughnessMap, cbufferData.RoughnessTexToggle, [&]()
                {
                    ImGui::PushItemWidth(-1);
                    ImGui::SliderFloat("##RoughnessData", &cbufferData.Roughness, 0, 1);
                    ImGui::PopItemWidth();
                });

                DrawMaterialProperty("Normal", material, material->mNormalMap, cbufferData.NormalTexToggle, [&](){});

                DrawMaterialProperty("Ambient Occlusion", material, material->mAOMap, cbufferData.AOTexToggle, [&]()
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
