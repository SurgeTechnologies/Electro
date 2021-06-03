//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "RendererSettingsPanel.hpp"
#include "Renderer/SceneRenderer.hpp"
#include "Renderer/Cascades.hpp"
#include "Renderer/RendererDebug.hpp"
#include "UIMacros.hpp"
#include "UIUtils/UiUtils.hpp"
#include <imgui.h>
#include <imgui_internal.h>

namespace Electro
{
    void RendererSettingsPanel::OnImGuiRender(bool show)
    {
        const Scope<SceneRendererData>& rendererData = SceneRenderer::GetData();

        ImGui::Begin(RENDERER_SETTINGS_TITLE, &show);
        if (ImGui::CollapsingHeader("Environment"))
        {
            ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV;
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            if (ImGui::BeginTable("EnvMapTable", 2, flags))
            {
                ImGui::TableSetupColumn("##col1", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                ImGui::TableSetupColumn("##col2", ImGuiTableColumnFlags_WidthFixed, contentRegionAvailable.x * 0.6156f);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Path");
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);
                Ref<EnvironmentMap>& environmentMap = SceneRenderer::GetEnvironmentMapSlot();
                if (environmentMap && !environmentMap->GetPath().empty())
                    ImGui::InputText("##envfilepath", (char*)environmentMap->GetPath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
                else
                    ImGui::InputText("##envfilepath", (char*)"", 256, ImGuiInputTextFlags_ReadOnly);

                const ImGuiPayload* dropData = UI::DragAndDropTarget(TEXTURE_DND_ID);
                if (dropData)
                    environmentMap = Factory::CreateEnvironmentMap(*(String*)dropData->Data);
                ImGui::EndTable();

                if (environmentMap)
                {
                    bool remove = false;
                    ImGui::SameLine();
                    if (ImGui::Button("Remove"))
                    {
                        // Unbind the Irradiance & Prefilter Map
                        environmentMap->GetCubemap()->Unbind(5);
                        environmentMap->GetCubemap()->Unbind(6);
                        environmentMap.Reset();
                        remove = true;
                    }
                    if (!remove)
                    {
                        ImGui::SameLine();
                        if (ImGui::Checkbox("##UseEnvMap", &SceneRenderer::GetEnvironmentMapActivationBool()))
                        {
                            if (!SceneRenderer::GetEnvironmentMapActivationBool())
                            {
                                environmentMap->GetCubemap()->Unbind(5);
                                environmentMap->GetCubemap()->Unbind(6);
                            }
                            else
                            {
                                environmentMap->GetCubemap()->BindIrradianceMap(5);
                                environmentMap->GetCubemap()->BindPreFilterMap(6);
                            }
                        }
                        UI::ToolTip("Use Environment Map");
                        UI::SliderFloat("Skybox LOD", environmentMap->mTextureLOD, 0.0f, 11.0f);
                        UI::SliderFloat("Intensity", environmentMap->mIntensity, 1.0f, 100.0f);
                    }
                }
            }
        }
        if (ImGui::CollapsingHeader("Shaders"))
        {
            Vector<Ref<Shader>>& shaders = AssetManager::GetAll<Shader>(AssetType::Shader);
            for (Ref<Shader>& shader : shaders)
            {
                ImGui::PushID(shader->GetName().c_str());
                if (ImGui::TreeNode(shader->GetName().c_str()))
                {
                    if (ImGui::Button("Reload"))
                        shader->Reload();
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        }
        if (ImGui::CollapsingHeader("Materials"))
        {
            Vector<Ref<Material>>& mats = AssetManager::GetAll<Material>(AssetType::Material);
            for (Ref<Material>& mat : mats)
            {
                ImGui::PushID(mat->GetName().c_str());
                if (ImGui::TreeNode(mat->GetName().c_str()))
                    ImGui::TreePop();
                ImGui::PopID();
            }
        }
        if (ImGui::CollapsingHeader("Debug"))
        {
            const Pair<bool*, bool*> debugData = RendererDebug::GetToggles();
            UI::Checkbox("Show Grid", debugData.Data1, 160.0f);
            UI::Checkbox("Show Camera Frustum", debugData.Data2, 160.0f);

            ImGui::PushItemWidth(-1);
            ImGui::TextUnformatted("Cascade Index");
            ImGui::SameLine();
            ImGui::SliderInt("##CascadeIndexSlider", &mIndex, 0, NUM_CASCADES - 1);
            ImGui::PopItemWidth();

            if (ImGui::TreeNode("Shadow Map"))
            {
                ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
                ImGui::DragFloat("##a", &mImageSize.x);
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::TextUnformatted("X");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::DragFloat("##b", &mImageSize.y);
                ImGui::PopItemWidth();
                ImGui::Image(static_cast<ImTextureID>(rendererData->ShadowMapCascades.GetFramebuffers()[mIndex]->GetDepthAttachmentID()), ImVec2(mImageSize.x, mImageSize.y));
                ImGui::TreePop();
            }
        }
        ImGui::End();
    }
}