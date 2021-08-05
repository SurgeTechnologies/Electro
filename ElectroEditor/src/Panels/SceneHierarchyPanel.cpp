//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "SceneHierarchyPanel.hpp"
#include "Core/Input.hpp"
#include "UIUtils/UIUtils.hpp"
#include "EditorLayer.hpp"

namespace Electro
{
    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
    {
        SetContext(context);
    }

    void SceneHierarchyPanel::OnInit(void* data)
    {
        mEditorLayer = static_cast<EditorLayer*>(data);
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
        ImGui::Begin(HIERARCHY_TITLE, show);

        if (ImGui::Button("Add Entity", { ImGui::GetWindowWidth(), 0.0f }))
            ImGui::OpenPopup("Add Entity");

        if (ImGui::BeginPopup("Add Entity") || ImGui::BeginPopupContextWindow(nullptr, 1, false))
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
            if (ImGui::MenuItem("Mesh"))
            {
                mSelectionContext = mContext->CreateEntity("Mesh");
                mSelectionContext.AddComponent<MeshComponent>();
            }
            if (ImGui::BeginMenu("Lights"))
            {
                if (ImGui::MenuItem("PointLight"))
                {
                    mSelectionContext = mContext->CreateEntity("Point Light");
                    mSelectionContext.AddComponent<PointLightComponent>();
                }
                if (ImGui::MenuItem("DirectionalLight"))
                {
                    mSelectionContext = mContext->CreateEntity("Directional Light");
                    mSelectionContext.AddComponent<DirectionalLightComponent>();
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        bool sceneNodeOpened = ImGui::TreeNodeEx(static_cast<void*>(&mContext->GetUUID()), 
            ImGuiTreeNodeFlags_DefaultOpen,
            fmt::format("{0} - {1}", mContext->GetName().c_str(), mContext->GetUUID()).c_str());

        {
            const ImGuiPayload* data = UI::DragAndDropTarget(ELECTRO_SCENE_FILE_DND_ID);
            if (data)
            {
                const String filepath = *static_cast<String*>(data->Data);
                mEditorLayer->InitSceneEssentials(FileSystem::GetNameWithoutExtension(filepath));
                mEditorLayer->DeserializeScene(filepath);
            }
        }

        if(sceneNodeOpened)
        {
            // For each entity in the registry, draw it
            mContext->GetRegistry().each([&](auto entityID)
            {
                Entity entity{ entityID, mContext.Raw() };
                if (entity.HasComponent<IDComponent>())
                    DrawEntityNode(entity);
            });

            ImGui::TreePop();
        }

        mIsHierarchyFocused = ImGui::IsWindowFocused();
        mIsHierarchyHovered = ImGui::IsWindowHovered();

        if (ImGui::IsMouseDown(0) && mIsHierarchyHovered)
            mSelectionContext = {};

        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        String& tag = entity.GetComponent<TagComponent>().Tag;
        ImGuiTreeNodeFlags flags = ((mSelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        if (mContext->GetSelectedEntity() == entity)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.1f, 1.0f));

        const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uint64_t>(static_cast<uint32_t>(entity))), flags, tag.c_str());

        if (mContext->GetSelectedEntity() == entity)
            ImGui::PopStyleColor();

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
}
