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
                    if (ImGui::IsItemHovered())
                    {
                        if (material->mAlbedoMap)
                        {
                            ImGui::BeginTooltip();
                            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                            ImGui::TextUnformatted(material->mAlbedoMap->GetFilepath().c_str());
                            ImGui::PopTextWrapPos();
                            ImGui::Image((void*)material->mAlbedoMap->GetRendererID(), ImVec2(384, 384));
                            ImGui::EndTooltip();
                        }
                        if (ImGui::IsItemClicked())
                        {
                            auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                            if (filename)
                                material->mAlbedoMap = Texture2D::Create(*filename, true);
                        }
                    }
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    if (ImGui::Checkbox("Use##AlbedoMap", &useAlbedoMap))
                        bufferData.AlbedoTexToggle = useAlbedoMap;

                    ImGui::EndGroup();
                    ImGui::SameLine();
                    ImGui::ColorEdit3("Color##Albedo", glm::value_ptr(albedoColor), ImGuiColorEditFlags_NoInputs);
                }

                //Metallic
                if (ImGui::CollapsingHeader("Metalness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                    auto& metallicColor = bufferData.Metallic;
                    bool useMetallicMap = bufferData.MetallicTexToggle;
                    UI::Image(material->mMetallicMap ? (void*)material->mMetallicMap->GetRendererID() : nullptr, { 64, 64 });
                    ImGui::PopStyleVar();
                    if (ImGui::IsItemHovered())
                    {
                        if (material->mMetallicMap)
                        {
                            ImGui::BeginTooltip();
                            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                            ImGui::TextUnformatted(material->mMetallicMap->GetFilepath().c_str());
                            ImGui::PopTextWrapPos();
                            ImGui::Image((void*)material->mMetallicMap->GetRendererID(), ImVec2(384, 384));
                            ImGui::EndTooltip();
                        }
                        if (ImGui::IsItemClicked())
                        {
                            auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                            if (filename)
                                material->mMetallicMap = Texture2D::Create(*filename, true);
                        }
                    }
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    if (ImGui::Checkbox("Use##MetallicMap", &useMetallicMap))
                        bufferData.MetallicTexToggle = useMetallicMap;

                    ImGui::EndGroup();
                    ImGui::SameLine();
                    UI::SliderFloat("Metalness", bufferData.Metallic, 0, 1, 150.0f);
                }

                //Normal
                if (ImGui::CollapsingHeader("Normal", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                    bool useNormalMap = bufferData.NormalTexToggle;
                    UI::Image(material->mNormalMap ? (void*)material->mNormalMap->GetRendererID() : nullptr, { 64, 64 });
                    ImGui::PopStyleVar();
                    if (ImGui::IsItemHovered())
                    {
                        if (material->mNormalMap)
                        {
                            ImGui::BeginTooltip();
                            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                            ImGui::TextUnformatted(material->mNormalMap->GetFilepath().c_str());
                            ImGui::PopTextWrapPos();
                            ImGui::Image((void*)material->mNormalMap->GetRendererID(), ImVec2(384, 384));
                            ImGui::EndTooltip();
                        }
                        if (ImGui::IsItemClicked())
                        {
                            auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                            if (filename)
                                material->mNormalMap = Texture2D::Create(*filename, true);
                        }
                    }
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    if (ImGui::Checkbox("Use##NormalMap", &useNormalMap))
                        bufferData.NormalTexToggle = useNormalMap;
                
                    ImGui::EndGroup();
                }

                //Roughness
                if (ImGui::CollapsingHeader("Roughness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                    auto& roughnessColor = bufferData.Roughness;
                    bool useRoughnessMap = bufferData.RoughnessTexToggle;
                    UI::Image(material->mRoughnessMap ? (void*)material->mRoughnessMap->GetRendererID() : nullptr, { 64, 64 });
                    ImGui::PopStyleVar();
                    if (ImGui::IsItemHovered())
                    {
                        if (material->mRoughnessMap)
                        {
                            ImGui::BeginTooltip();
                            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                            ImGui::TextUnformatted(material->mRoughnessMap->GetFilepath().c_str());
                            ImGui::PopTextWrapPos();
                            ImGui::Image((void*)material->mRoughnessMap->GetRendererID(), ImVec2(384, 384));
                            ImGui::EndTooltip();
                        }
                        if (ImGui::IsItemClicked())
                        {
                            auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                            if (filename)
                                material->mRoughnessMap = Texture2D::Create(*filename, true);
                        }
                    }
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    if (ImGui::Checkbox("Use##RoughnessMap", &useRoughnessMap))
                        bufferData.RoughnessTexToggle = useRoughnessMap;

                    ImGui::EndGroup();
                    ImGui::SameLine();
                    UI::SliderFloat("Roughness", bufferData.Roughness, 0, 1, 150.0f);
                }

                //AO
                if (ImGui::CollapsingHeader("Ambient Occlusion", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
                    bool useAOMap = bufferData.AOTexToggle;
                    UI::Image(material->mAOMap ? (void*)material->mAOMap->GetRendererID() : nullptr, { 64, 64 });
                    ImGui::PopStyleVar();
                    if (ImGui::IsItemHovered())
                    {
                        if (material->mAOMap)
                        {
                            ImGui::BeginTooltip();
                            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                            ImGui::TextUnformatted(material->mAOMap->GetFilepath().c_str());
                            ImGui::PopTextWrapPos();
                            ImGui::Image((void*)material->mAOMap->GetRendererID(), ImVec2(384, 384));
                            ImGui::EndTooltip();
                        }
                        if (ImGui::IsItemClicked())
                        {
                            auto filename = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                            if (filename)
                                material->mAOMap = Texture2D::Create(*filename, true);
                        }
                    }
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    if (ImGui::Checkbox("Use##AOMap", &useAOMap))
                        bufferData.AOTexToggle = useAOMap;

                    ImGui::EndGroup();
                    ImGui::SameLine();
                    UI::Float("AO", &bufferData.AO, 150.0f);
                }
            }
        }
        ImGui::End();
    }
}
