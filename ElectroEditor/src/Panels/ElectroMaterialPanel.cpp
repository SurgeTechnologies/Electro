//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroMaterialPanel.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include "ElectroVaultPanel.hpp"
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
            Ref<Material> material;
            if (mesh)
            {
                material = mesh->GetMaterial();
                auto& bufferData = material->GetCBufferData();
                ImGui::TextColored(ImVec4(0.1f, 0.9f, 0.1f, 1.0f), "Shader: %s", material->GetShader()->GetName().c_str());
                ImGui::Separator();

                //Albedo
                if (ImGui::CollapsingHeader("Albedo", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                    auto& albedoColor = bufferData.Albedo;
                    bool useAlbedoMap = bufferData.AlbedoTexToggle;
                    UI::Image(material->mAlbedoMap ? (void*)material->mAlbedoMap->GetRendererID() : nullptr, { 64, 64 });
                    ImGui::PopStyleVar();
                    if (ImGui::IsItemClicked())
                    {
                        auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                        if (filename)
                        {
                            material->mAlbedoMap = Texture2D::Create(*filename);
                            if(material->mAlbedoMap)
                                bufferData.AlbedoTexToggle = true;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Use##AlbedoMap", &useAlbedoMap))
                        bufferData.AlbedoTexToggle = useAlbedoMap;

                    ImGui::SameLine();
                    if (ImGui::Button("Preview##AlbedoMap") && material->mAlbedoMap)
                    {
                        GetTexturePreviewtorage() = material->mAlbedoMap;
                        ImGui::SetWindowFocus("Texture Preview");
                    }

                    ImGui::SameLine();
                    ImGui::ColorEdit3("Raw Color##Albedo", glm::value_ptr(albedoColor), ImGuiColorEditFlags_NoInputs);
                }

                //Metallic
                if (ImGui::CollapsingHeader("Metalness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                    bool useMetallicMap = bufferData.MetallicTexToggle;
                    UI::Image(material->mMetallicMap ? (void*)material->mMetallicMap->GetRendererID() : nullptr, { 64, 64 });
                    ImGui::PopStyleVar();
                    if (ImGui::IsItemClicked() && ImGui::IsItemHovered())
                    {
                        auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                        if (filename)
                        {
                            material->mMetallicMap = Texture2D::Create(*filename);
                            if(material->mMetallicMap)
                                bufferData.MetallicTexToggle = true;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Use##MetallicMap", &useMetallicMap))
                        bufferData.MetallicTexToggle = useMetallicMap;

                    ImGui::SameLine();
                    if (ImGui::Button("Preview##MetalnessMap") && material->mMetallicMap)
                    {
                        GetTexturePreviewtorage() = material->mMetallicMap;
                        ImGui::SetWindowFocus("Texture Preview");
                    }

                    UI::SliderFloat("Metalness Value", bufferData.Metallic, 0, 1, 150.0f);
                }

                //Normal
                if (ImGui::CollapsingHeader("Normal", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                    bool useNormalMap = bufferData.NormalTexToggle;
                    UI::Image(material->mNormalMap ? (void*)material->mNormalMap->GetRendererID() : nullptr, { 64, 64 });
                    ImGui::PopStyleVar();
                    if (ImGui::IsItemClicked())
                    {
                        auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                        if (filename)
                        {
                            material->mNormalMap = Texture2D::Create(*filename);
                            if(material->mNormalMap)
                                bufferData.NormalTexToggle = true;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Use##NormalMap", &useNormalMap))
                        bufferData.NormalTexToggle = useNormalMap;

                    ImGui::SameLine();
                    if (ImGui::Button("Preview##NormalMap") && material->mNormalMap)
                    {
                        GetTexturePreviewtorage() = material->mNormalMap;
                        ImGui::SetWindowFocus("Texture Preview");
                    }
                }

                //Roughness
                if (ImGui::CollapsingHeader("Roughness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                    auto& roughnessColor = bufferData.Roughness;
                    bool useRoughnessMap = bufferData.RoughnessTexToggle;
                    UI::Image(material->mRoughnessMap ? (void*)material->mRoughnessMap->GetRendererID() : nullptr, { 64, 64 });
                    ImGui::PopStyleVar();
                    if (ImGui::IsItemClicked())
                    {
                        auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                        if (filename)
                        {
                            material->mRoughnessMap = Texture2D::Create(*filename);
                            if(material->mRoughnessMap)
                                bufferData.RoughnessTexToggle = true;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Use##RoughnessMap", &useRoughnessMap))
                        bufferData.RoughnessTexToggle = useRoughnessMap;

                    ImGui::SameLine();
                    if (ImGui::Button("Preview##RoughnessMap") && material->mRoughnessMap)
                    {
                        GetTexturePreviewtorage() = material->mRoughnessMap;
                        ImGui::SetWindowFocus("Texture Preview");
                    }
                    UI::SliderFloat("Roughness Value", bufferData.Roughness, 0, 1, 150.0f);
                }

                //AO
                if (ImGui::CollapsingHeader("Ambient Occlusion", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                    bool useAOMap = bufferData.AOTexToggle;
                    UI::Image(material->mAOMap ? (void*)material->mAOMap->GetRendererID() : nullptr, { 64, 64 });
                    ImGui::PopStyleVar();
                    if (ImGui::IsItemClicked())
                    {
                        auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                        if (filename)
                        {
                            material->mAOMap = Texture2D::Create(*filename);
                            if(material->mAOMap)
                                bufferData.AOTexToggle = true;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Use##AOMap", &useAOMap))
                        bufferData.AOTexToggle = useAOMap;

                    ImGui::SameLine();
                    if (ImGui::Button("Preview##AOMap") && material->mAOMap)
                    {
                        GetTexturePreviewtorage() = material->mAOMap;
                        ImGui::SetWindowFocus("Texture Preview");
                    }
                    UI::Float("AO Value", &bufferData.AO, 150.0f);
                }
            }
        }
        ImGui::End();
    }
}
