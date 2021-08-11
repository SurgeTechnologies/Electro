//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "RendererSettingsPanel.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Asset/AssetManager.hpp"
#include "UIMacros.hpp"
#include "UIUtils/UiUtils.hpp"
#include "AssetsPanel.hpp"
#include "AssetImportPopup.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include "Renderer/RenderPass/DebugPass.hpp"
#include "Renderer/RenderPass/ShadowPass.hpp"

namespace Electro
{
    void RendererSettingsPanel::OnInit(void* data)
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
                if (environmentMap)
                    ImGui::InputText("##envfilepath", (char*)environmentMap->GetPath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
                else
                    ImGui::InputText("##envfilepath", (char*)"", 256, ImGuiInputTextFlags_ReadOnly);

                const ImGuiPayload* droppedData = UI::DragAndDropTarget(TEXTURE_DND_ID);
                if (droppedData)
                {
                    AssetDropData assetDropData = *(AssetDropData*)droppedData->Data;
                    if (assetDropData.Handle != 0)
                    {
                        // Asset is present in registry, get that!
                        environmentMap = AssetManager::GetAsset<EnvironmentMap>(assetDropData.Handle);
                        if (environmentMap)
                            mRendererData->EnvironmentMapActivated = true;
                    }
                    else
                    {
                        // Handle is invalid, propt user to add texture as an asset
                        AssetImportPopup::ThrowImportPopup(AssetType::ENVIRONMENT_MAP, assetDropData.Path);
                    }
                }
                AssetImportPopup::CatchImportPopup(AssetType::ENVIRONMENT_MAP);
                ImGui::EndTable();

                if (environmentMap)
                {
                    bool remove = false;
                    if (ImGui::Button("Remove"))
                    {
                        // Unbind the Irradiance & Prefilter Map + remove the environment map
                        environmentMap->GetCubemap()->Unbind(5);
                        environmentMap->GetCubemap()->Unbind(6);
                        environmentMap.Reset();
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
            ShadowPass::InternalData* shadowPassData = mRendererData->RenderPassManager.GetRenderPassData<ShadowPass>();
            if (ImGui::BeginTable("ShadowSettings", 2, ImGuiTableFlags_Resizable))
            {
                ShadowPass* shadowPass = mRendererData->RenderPassManager.GetRenderPass<ShadowPass>();

                // Shadow Map Resolution
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Shadow Map Resolution");
                ImGui::TableNextColumn();
                ImGui::PushItemWidth(-1);
                if (ImGui::DragInt("##ShAdOwMaPrEs", reinterpret_cast<int*>(&shadowPassData->ShadowMapResolution)))
                    shadowPass->ResizeAllShadowMaps(shadowPassData->ShadowMapResolution);
                ImGui::PopItemWidth();

                // Cascade Split Lambda
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Cascade Split Lambda");
                ImGui::TableNextColumn();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##CSpL!tLd@", &shadowPassData->CascadeSplitLambda);
                ImGui::PopItemWidth();

                ImGui::EndTable();
            }
            if (ImGui::TreeNode("Shadow Map"))
            {
                ImGui::PushItemWidth(-1);
                ImGui::SliderInt("##CascadeIndex", &mCascadeIndex, 0, NUM_CASCADES - 1);
                ImGui::PopItemWidth();
                ImGui::Image(static_cast<ImTextureID>(shadowPassData->ShadowMaps[mCascadeIndex]->GetDepthAttachmentID()), ImVec2(200, 200));
                ImGui::TreePop();
            }
        }
        if (ImGui::CollapsingHeader("Bloom"))
        {
            Bloom* bloom = mRendererData->PostProcessPipeline.GetEffectByKey<Bloom>(BLOOM_METHOD_KEY);

            bool bloomEnabled = bloom->IsEnabled();
            if (UI::Checkbox("Enable Bloom", &bloomEnabled, 160.0f))
                bloom->SetEnabled(bloomEnabled);

            float bloomThreshold = bloom->GetBloomThreshold();
            if (UI::Float("Bloom Threshold", &bloomThreshold, 160.0f))
                bloom->SetBloomThreshold(bloomThreshold);

            float gaussianSigma = bloom->GetGaussianSigma();
            if (UI::Float("Gaussian Sigma", &gaussianSigma, 160.0f))
                bloom->SetGaussianSigma(gaussianSigma);

            if (ImGui::TreeNode("Blur Map"))
            {
                ImGui::Image(static_cast<ImTextureID>(Renderer::GetBloomBlurTexture()->GetColorAttachmentID(0)), ImVec2(200, 200));
                ImGui::TreePop();
            }
        }
        if (ImGui::CollapsingHeader("Composite"))
        {
            if (ImGui::BeginTable("CompoTable", 2, ImGuiTableFlags_Resizable))
            {
                // Exposure
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Exposure");
                ImGui::TableNextColumn();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##ExPosUre", &mRendererData->CompositeParams.Exposure);
                ImGui::PopItemWidth();

                // Tonemap Selection
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Tonemapping Algorithm");
                ImGui::TableNextColumn();
                ImGui::PushItemWidth(-1);
                const char* toneMapStrings[] = { "Reinhard", "Uncharted2", "ReinhardJodie", "ACESApprox", "ACESFitted" };
                UI::Dropdown(toneMapStrings, 5, (int32_t*)&mRendererData->CompositeParams.ToneMappingAlgorithm);
                ImGui::PopItemWidth();

                // FXAA
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("FXAA");
                ImGui::TableNextColumn();
                ImGui::Checkbox("##FxAa", reinterpret_cast<bool*>(&mRendererData->CompositeParams.ApplyFXAA));
                ImGui::EndTable();
            }
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
            DebugPass::InternalData* debugPassData = mRendererData->RenderPassManager.GetRenderPassData<DebugPass>();
            if (ImGui::BeginTable("DebugSettingsTable", 2, ImGuiTableFlags_Resizable))
            {
                // Show Grid
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Show Grid");
                ImGui::TableNextColumn();
                ImGui::Checkbox("##SHoWgrId", &debugPassData->ShowGrid);

                // Show Camera Frustum
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Show Camera Frustum");
                ImGui::TableNextColumn();
                ImGui::Checkbox("##fRusTum", &debugPassData->ShowCameraFrustum);

                // Show Camera Frustum
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Show BoundingBoxes");
                ImGui::TableNextColumn();
                ImGui::Checkbox("##shOwBB", &debugPassData->ShowAABB);

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
}