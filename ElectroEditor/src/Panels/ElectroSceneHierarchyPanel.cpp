//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroSceneHierarchyPanel.hpp"
#include "Core/ElectroInput.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Renderer/ElectroRendererAPISwitch.hpp"
#include "Scene/ElectroComponents.hpp"
#include "Scripting/ElectroScriptEngine.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <FontAwesome.hpp>
#include <stb_image.h>

namespace Electro
{
    template<typename T, typename UIFunction>
    static void DrawComponent(const String& name, Entity entity, UIFunction uiFunction)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
        if (entity.HasComponent<T>())
        {
            ImGui::PushID((void*)typeid(T).hash_code());
            auto& component = entity.GetComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();

            bool open = ImGui::TreeNodeEx("##dummy_id", treeNodeFlags, name.c_str());
            ImGui::PopStyleVar();

            ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
            if (ImGui::Button(ICON_ELECTRO_COG, ImVec2{ lineHeight, lineHeight }))
            {
                ImGui::OpenPopup("Component Settings");
            }

            bool removeComponent = false;
            if (ImGui::BeginPopup("Component Settings"))
            {
                if (ImGui::MenuItem("Reset"))
                    component.Reset();
                if (ImGui::MenuItem("Remove component"))
                    removeComponent = true;

                ImGui::EndPopup();
            }

            if (open)
            {
                uiFunction(component);
                ImGui::TreePop();
            }

            if (removeComponent)
                entity.RemoveComponent<T>();

            ImGui::PopID();
        }
    }

    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
    {
        SetContext(context);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
    {
        mContext = context;
        mSelectionContext = {};

        if (mSelectionContext && false)
        {
            // Try to find same entity in new scene
            auto& entityMap = mContext->GetEntityMap();
            UUID selectedEntityID = mSelectionContext.GetUUID();
            if (entityMap.find(selectedEntityID) != entityMap.end())
                mSelectionContext = entityMap.at(selectedEntityID);
        }
    }

    void SceneHierarchyPanel::OnImGuiRender(bool* show)
    {
        // Hierarchy
        ImGui::Begin("Hierarchy", show);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        if (ImGui::Button("Add Entity", { ImGui::GetWindowWidth(), 0.0f }))
            ImGui::OpenPopup("Add Entity");
        ImGui::PopStyleVar();

        if (ImGui::BeginPopup("Add Entity") || ImGui::BeginPopupContextWindow(0, 1, false))
        {
            if (ImGui::MenuItem("Empty Entity"))
            {
                mSelectionContext = mContext->CreateEntity("Entity");
            }
            if (ImGui::MenuItem("Camera"))
            {
                mSelectionContext = mContext->CreateEntity("Camera");
                mSelectionContext.AddComponent<CameraComponent>();
            }
            if (ImGui::MenuItem("Sprite"))
            {
                mSelectionContext = mContext->CreateEntity("Sprite");
                mSelectionContext.AddComponent<SpriteRendererComponent>();
            }
            if (ImGui::MenuItem("Mesh"))
            {
                mSelectionContext = mContext->CreateEntity("Mesh");
                mSelectionContext.AddComponent<MeshComponent>();
            }
            if (ImGui::MenuItem("Directional Light"))
            {
                mSelectionContext = mContext->CreateEntity("Directional Light");
                mSelectionContext.AddComponent<SkyLightComponent>();
            }
            if (ImGui::MenuItem("PointLight"))
            {
                mSelectionContext = mContext->CreateEntity("Point Light");
                mSelectionContext.AddComponent<PointLightComponent>();
            }

            ImGui::EndPopup();
        }

        //For each entity in the registry, draw it!
        mContext->mRegistry.each([&](auto entityID)
        {
            Entity entity{ entityID, mContext.Raw() };
            if (entity.HasComponent<IDComponent>())
                DrawEntityNode(entity);
        });

        mIsHierarchyFocused = ImGui::IsWindowFocused();
        mIsHierarchyHovered = ImGui::IsWindowHovered();

        if (ImGui::IsMouseDown(0) && mIsHierarchyHovered)
            mSelectionContext = {};

        ImGui::End();

        // Inspector
        ImGui::Begin(ICON_ELECTRO_INFO_CIRCLE" Inspector", show);
        if (mSelectionContext)
            DrawComponents(mSelectionContext);

        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = ((mSelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

        if (ImGui::IsItemClicked())
            mSelectionContext = entity;

        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            ImGui::EndPopup();
        }

        if (opened)
            ImGui::TreePop();

        if (entityDeleted)
        {
            mContext->DestroyEntity(entity);
            if (mSelectionContext == entity)
                mSelectionContext = {};
        }

    }

    void SceneHierarchyPanel::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(ELECTRO_BIND_EVENT_FN(SceneHierarchyPanel::OnKeyPressed));
    }

    bool SceneHierarchyPanel::OnKeyPressed(KeyPressedEvent& e)
    {
        if (Input::IsKeyPressed(Key::Delete) && mIsHierarchyFocused)
        {
            if (mSelectionContext)
                mContext->DestroyEntity(mSelectionContext);
            mSelectionContext = {};
            return false;
        }
        if ((Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl)) && Input::IsKeyPressed(Key::D))
            mContext->DuplicateEntity(mSelectionContext);

        return false;
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        auto ID = entity.GetComponent<IDComponent>().ID;

        if (entity.HasComponent<TagComponent>())
            UI::DrawTextControlWithoutLabel(&entity.GetComponent<TagComponent>().Tag);

        ImGui::TextDisabled("UUID: %llx", entity.GetComponent<IDComponent>().ID);
        DrawComponent<TransformComponent>(ICON_ELECTRO_ARROWS_ALT" Transform", entity, [](auto& component)
        {
            UI::DrawVec3Control("Translation", component.Translation);
            glm::vec3 rotation = glm::degrees(component.Rotation);
            UI::DrawVec3Control("Rotation", rotation);
            component.Rotation = glm::radians(rotation);
            UI::DrawVec3Control("Scale", component.Scale, 1.0f);
        });

        DrawComponent<CameraComponent>(ICON_ELECTRO_CAMERA" Camera", entity, [](auto& component)
        {
            auto& camera = component.Camera;
            UI::DrawBoolControl("Primary", &component.Primary, 160.0f);

            const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
            const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
            ImGui::Columns(2);
            ImGui::Text("Projection");
            ImGui::SetColumnWidth(0, 160.0f);
            ImGui::NextColumn();
            if (ImGui::BeginCombo("##Projection", currentProjectionTypeString))
            {
                for (int i = 0; i < 2; i++)
                {
                    bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                    if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
                    {
                        currentProjectionTypeString = projectionTypeStrings[i];
                        camera.SetProjectionType((SceneCamera::ProjectionType)i);
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Columns(1);
            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
            {
                float verticalFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
                if (UI::DrawFloatControl("Vertical FOV", &verticalFOV, 160.0f))
                    camera.SetPerspectiveVerticalFOV(glm::radians(verticalFOV));

                float nearClip = camera.GetPerspectiveNearClip();
                if (UI::DrawFloatControl("Near Clip", &nearClip, 160.0f))
                    camera.SetPerspectiveNearClip(nearClip);

                float farClip = camera.GetPerspectiveFarClip();
                if (UI::DrawFloatControl("Far Clip", &farClip, 160.0f))
                    camera.SetPerspectiveFarClip(farClip);
            }

            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
            {
                float orthoSize = camera.GetOrthographicSize();
                if (UI::DrawFloatControl("Size", &orthoSize, 160.0f))
                    camera.SetOrthographicSize(orthoSize);

                float nearClip = camera.GetOrthographicNearClip();
                if (UI::DrawFloatControl("Near Clip", &nearClip, 160.0f))
                    camera.SetOrthographicNearClip(nearClip);

                float farClip = camera.GetOrthographicFarClip();
                if (UI::DrawFloatControl("Far Clip", &farClip, 160.0f))
                    camera.SetOrthographicFarClip(farClip);

                UI::DrawBoolControl("Fixed Aspect Ratio", &component.FixedAspectRatio, 160.0f);
            }
        });

        DrawComponent<SpriteRendererComponent>(ICON_ELECTRO_SQUARE" Sprite Renderer", entity, [](auto& component)
        {
            UI::DrawColorControl4("Color", component.Color);

            const RendererID imageID = component.Texture.Raw() == nullptr ? 0 : component.Texture->GetRendererID();

            ImGui::Text("Texture");
            const float cursorPos = ImGui::GetCursorPosY();
            ImGui::SameLine(ImGui::GetWindowWidth() * 0.8f);

            if(UI::DrawImageButtonControl(imageID, { 65, 65 }))
            {
                char const* lFilterPatterns[8] = { "*.png", "*.jpg", "*.tga", "*.bmp", "*.psd", "*.hdr", "*.pic", "*.gif" };
                const char* filepath = OS::OpenFile("Open Texture", "", 8, lFilterPatterns, "", false);
                if (filepath)
                    component.SetTexture(filepath);
            }

            ImGui::SetCursorPosY(cursorPos + 5);

            if (ImGui::Button("Open Texture"))
            {
                char const* lFilterPatterns[8] = { "*.png", "*.jpg", "*.tga", "*.bmp", "*.psd", "*.hdr", "*.pic", "*.gif" };
                const char* filepath = OS::OpenFile("Open Texture", "", 8, lFilterPatterns, "", false);
                if (filepath)
                    component.SetTexture(filepath);
            }

            ImGui::SameLine();

            if (ImGui::Button("Remove Texture"))
                component.RemoveTexture();

            // Tiling Factor
            UI::DrawFloatControl("Tiling Factor", &component.TilingFactor, 100);
        });

        DrawComponent<MeshComponent>(ICON_ELECTRO_CUBE" Mesh", entity, [](auto& component)
        {
            ImGui::Text("File Path");
            ImGui::SameLine();
            if (component.Mesh)
                ImGui::InputText("##meshfilepath", (char*)component.Mesh->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
            else
                ImGui::InputText("##meshfilepath", (char*)"", 256, ImGuiInputTextFlags_ReadOnly);

            if (ImGui::Button("Open"))
            {
                const char* patterns[4] = { "*.fbx", "*.obj", "*.max", "*.3ds" };
                const char* file = OS::OpenFile("Open 3D Object file", "", 4, patterns, "", false);
                if (file)
                {
                    component.Mesh = Ref<Mesh>::Create(file);
                    component.SetFilePath(String(file));
                }
            }
            if (component.Mesh)
            {
                ImGui::SameLine();
                bool remove = false;
                if (ImGui::Button("Remove"))
                {
                    component.Reset();
                    remove = true;
                }
            }
        });

        DrawComponent<PointLightComponent>(ICON_ELECTRO_LIGHTBULB_O" PointLight", entity, [](auto& component)
        {
            UI::DrawColorControl3("Color", component.Color);
            UI::DrawFloatControl("Intensity", &component.Intensity);
            UI::DrawFloatControl("Constant", &component.Constant);
            UI::DrawFloatControl("Linear", &component.Linear);
            UI::DrawFloatControl("Quadratic", &component.Quadratic);
        });

        DrawComponent<SkyLightComponent>(ICON_ELECTRO_SUN_O" SkyLight", entity, [](auto& component)
        {
            UI::DrawFloatControl("Intensity", &component.Intensity);
            UI::DrawColorControl3("Color", component.Color);
        });

        DrawComponent<ScriptComponent>(ICON_ELECTRO_CODE" Script", entity, [=](auto& component) mutable
        {
            if (UI::DrawScriptTextControl("Module Name", component.ModuleName, 100.0f, ScriptEngine::ModuleExists(component.ModuleName)))
            {
                if (ScriptEngine::ModuleExists(component.ModuleName))
                    ScriptEngine::InitScriptEntity(entity);
            }
        });

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
        ImGui::SetCursorPosX(static_cast<float>(ImGui::GetWindowWidth() / 2.5));

        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("Add Component");

        if (ImGui::BeginPopup("Add Component"))
        {
            if (ImGui::BeginMenu("Rendering"))
            {
                if (ImGui::MenuItem("Camera"))
                {
                    if (!entity.HasComponent<CameraComponent>())
                        entity.AddComponent<CameraComponent>();
                    else
                        ELECTRO_WARN("This entity already has Camera component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Sprite Renderer"))
                {
                    if (!entity.HasComponent<SpriteRendererComponent>())
                        entity.AddComponent<SpriteRendererComponent>();
                    else
                        ELECTRO_WARN("This entity already has SpriteRenderer component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Mesh"))
                {
                    if (!entity.HasComponent<MeshComponent>())
                        entity.AddComponent<MeshComponent>();
                    else
                        ELECTRO_WARN("This entity already has Mesh component!");
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Lights"))
            {
                if (ImGui::MenuItem("PointLight"))
                {
                    if (!entity.HasComponent<PointLightComponent>())
                        entity.AddComponent<PointLightComponent>();
                    else
                        ELECTRO_WARN("This entity already has PointLight component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("SkyLight"))
                {
                    if (!entity.HasComponent<SkyLightComponent>())
                        entity.AddComponent<SkyLightComponent>();
                    else
                        ELECTRO_WARN("This entity already has SkyLight component!");
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Script"))
            {
                if (!entity.HasComponent<ScriptComponent>())
                    entity.AddComponent<ScriptComponent>();
                else
                    ELECTRO_WARN("This entity already has Script component!");
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}