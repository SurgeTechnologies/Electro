//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "RendererSettingsPanel.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Shadows.hpp"
#include "Renderer/Renderer2D.hpp"
#include "UIMacros.hpp"
#include "UIUtils/UiUtils.hpp"
#include <imgui.h>
#include <imgui_internal.h>

namespace Electro
{
    void RendererSettingsPanel::Init(void* data)
    {
        mRendererData = Renderer::GetData().get();
    }

    void RendererSettingsPanel::OnImGuiRender(bool* show)
    {
        ImGui::Begin(RENDERER_SETTINGS_TITLE, show);

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
                Ref<EnvironmentMap>& environmentMap = mRendererData->EnvironmentMap;
                if (environmentMap && !environmentMap->GetPath().empty())
                    ImGui::InputText("##envfilepath", (char*)environmentMap->GetPath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
                else
                    ImGui::InputText("##envfilepath", (char*)"", 256, ImGuiInputTextFlags_ReadOnly);

                const ImGuiPayload* dropData = UI::DragAndDropTarget(TEXTURE_DND_ID);
                if (dropData)
                    environmentMap = EnvironmentMap::Create(*(String*)dropData->Data);
                ImGui::EndTable();

                if (environmentMap)
                {
                    bool remove = false;
                    if (ImGui::Button("Remove"))
                    {
                        // Unbind the Irradiance & Prefilter Map
                        environmentMap->GetCubemap()->Unbind(5);
                        environmentMap->GetCubemap()->Unbind(6);
                        remove = true;
                    }
                    if (!remove)
                    {
                        ImGui::SameLine();
                        if (ImGui::Checkbox("##UseEnvMap", &mRendererData->EnvironmentMapActivated))
                        {
                            if (!mRendererData->EnvironmentMapActivated)
                            {
                                // Unbind Prefilter and Irradiance map
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
                        UI::Float("Intensity", &environmentMap->mIntensity);
                    }
                }
            }
        }
        if (ImGui::CollapsingHeader("Shadows"))
        {
            UI::Checkbox("Enable Shadows", &mRendererData->ShadowsEnabled, 160.0f);
            {
                int shadowMapResolution = mRendererData->Shadows.GetShadowMapResolution();
                if (UI::Int("Shadow Map Resolution", &shadowMapResolution, 160.0f))
                    mRendererData->Shadows.Resize(shadowMapResolution);
            }
            {
                float cascadeSplitLambda = mRendererData->Shadows.GetCascadeSplitLambda();
                if (UI::Float("Cascade Split Lambda", &cascadeSplitLambda, 160.0f))
                    mRendererData->Shadows.SetCascadeSplitLambda(cascadeSplitLambda);
            }
            if (ImGui::TreeNode("Shadow Map"))
            {
                ImGui::SliderInt("##CascadeIndex", &mCascadeIndex, 0, NUM_CASCADES - 1);
                ImGui::Image(static_cast<ImTextureID>(mRendererData->Shadows.GetFramebuffers()[mCascadeIndex]->GetDepthAttachmentID()), ImVec2(200, 200));
                ImGui::TreePop();
            }
        }
        if (ImGui::CollapsingHeader("Bloom"))
        {
            UI::Checkbox("Enable Bloom", &mRendererData->BloomEnabled, 160.0f);
            UI::Float("Bloom Threshold", &mRendererData->BloomThreshold, 160.0f);
            UI::Float("Bloom Exposure", &mRendererData->BloomExposure, 160.0f);
        }
        if (ImGui::CollapsingHeader("Shaders"))
        {
            Vector<Ref<Shader>>& shaders = Renderer::GetAllShaders();
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
        if (ImGui::CollapsingHeader("Debug"))
        {
            UI::Checkbox("Show Grid", &mRendererData->ShowGrid, 160.0f);
            UI::Checkbox("Show Camera Frustum", &mRendererData->ShowCameraFrustum, 160.0f);
            UI::Checkbox("Show BoundingBoxes", &mRendererData->ShowAABB, 160.0f);
        }
        ImGui::End();
    }
}