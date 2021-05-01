//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroSceneHierarchyPanel.hpp"
#include "Core/ElectroInput.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Renderer/ElectroRendererAPISwitch.hpp"
#include "Renderer/ElectroMeshFactory.hpp"
#include "Scene/ElectroComponents.hpp"
#include "Scripting/ElectroScriptEngine.hpp"
#include "Physics/ElectroPhysXInternal.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include "ElectroUIMacros.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <FontAwesome.hpp>
#include <stb_image.h>
#include <future>

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

    void SceneHierarchyPanel::Init()
    {
        mPrototypeTextureID = Vault::Get<Texture2D>("Prototype.png")->GetRendererID();
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
        ImGui::TextDisabled("Scene ID: %llu", mContext->GetUUID());
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
        ImGui::Begin(INSPECTOR_TITLE, show);
        if (mSelectionContext)
            DrawComponents(mSelectionContext);

        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        String& tag = entity.GetComponent<TagComponent>().Tag;
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
            UI::TextWithoutLabel(&entity.GetComponent<TagComponent>().Tag);

        ImGui::TextDisabled("UUID: %llx", entity.GetComponent<IDComponent>().ID);
        DrawComponent<TransformComponent>(ICON_ELECTRO_ARROWS_ALT" Transform", entity, [](TransformComponent& component)
        {
            UI::Float3("Translation", component.Translation);
            glm::vec3 rotation = glm::degrees(component.Rotation);
            UI::Float3("Rotation", rotation);
            component.Rotation = glm::radians(rotation);
            UI::Float3("Scale", component.Scale, 1.0f);
        });

        DrawComponent<CameraComponent>(ICON_ELECTRO_CAMERA" Camera", entity, [](CameraComponent& component)
        {
            auto& camera = component.Camera;
            UI::Checkbox("Primary", &component.Primary, 160.0f);
            const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
            const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
            ImGui::Columns(2);
            ImGui::Text("Projection");
            ImGui::SetColumnWidth(0, 160.0f);
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
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
            ImGui::PopItemWidth();

            ImGui::Columns(1);
            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
            {
                float verticalFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
                if (UI::Float("Vertical FOV", &verticalFOV, 160.0f))
                    camera.SetPerspectiveVerticalFOV(glm::radians(verticalFOV));

                float nearClip = camera.GetPerspectiveNearClip();
                if (UI::Float("Near Clip", &nearClip, 160.0f))
                    camera.SetPerspectiveNearClip(nearClip);

                float farClip = camera.GetPerspectiveFarClip();
                if (UI::Float("Far Clip", &farClip, 160.0f))
                    camera.SetPerspectiveFarClip(farClip);
            }

            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
            {
                float orthoSize = camera.GetOrthographicSize();
                if (UI::Float("Size", &orthoSize, 160.0f))
                    camera.SetOrthographicSize(orthoSize);

                float nearClip = camera.GetOrthographicNearClip();
                if (UI::Float("Near Clip", &nearClip, 160.0f))
                    camera.SetOrthographicNearClip(nearClip);

                float farClip = camera.GetOrthographicFarClip();
                if (UI::Float("Far Clip", &farClip, 160.0f))
                    camera.SetOrthographicFarClip(farClip);

                UI::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio, 160.0f);
            }
        });

        DrawComponent<SpriteRendererComponent>(ICON_ELECTRO_SQUARE" Sprite Renderer", entity, [&](SpriteRendererComponent& component)
        {
            UI::Color4("Color", component.Color);

            ImGui::Text("Texture");
            const float cursorPos = ImGui::GetCursorPosY();
            ImGui::SameLine(ImGui::GetWindowWidth() * 0.8f);

            if(UI::ImageButton(component.Texture ? component.Texture->GetRendererID() : mPrototypeTextureID, { 50, 50 }))
            {
                ELECTRO_WARN("Renderer2D is not completely available in a 3D scene; that is currently Renderer2D is not fully functional.");
                auto filepath = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                if (filepath)
                    component.SetTexture(*filepath);
            }
            auto dropData = UI::DragAndDropTarget(TEXTURE_DND_ID);
            if (dropData)
            {
                ELECTRO_WARN("Renderer2D is not completely available in a 3D scene; that is currently Renderer2D is not fully functional.");
                component.SetTexture(*(String*)dropData->Data);
            }

            ImGui::SetCursorPosY(cursorPos + 5);

            if (ImGui::Button("Open Texture"))
            {
                ELECTRO_WARN("Renderer2D is not completely available in a 3D scene; that is currently Renderer2D is not fully functional.");
                auto filepath = OS::OpenFile("*.png; *.jpg; *.tga; *.bmp; *.psd; *.hdr; *.pic; *.gif\0");
                if (filepath)
                    component.SetTexture(*filepath);
            }

            ImGui::SameLine();

            if (ImGui::Button("Remove Texture"))
                component.RemoveTexture();

            // Tiling Factor
            UI::Float("Tiling Factor", &component.TilingFactor, 100);
        });

        DrawComponent<MeshComponent>(ICON_ELECTRO_CUBE" Mesh", entity, [](MeshComponent& component)
        {
            ImGui::Text("File Path");
            ImGui::SameLine();
            if (component.Mesh)
                ImGui::InputText("##meshfilepath", (char*)component.Mesh->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
            else
                ImGui::InputText("##meshfilepath", (char*)"", 256, ImGuiInputTextFlags_ReadOnly);
            auto dropData = UI::DragAndDropTarget(MESH_DND_ID);
            if (dropData)
                component.Mesh = EGenerator::CreateMesh(*(String*)dropData->Data);

            if (ImGui::Button("Open"))
            {
                auto file = OS::OpenFile("ObjectFile (*.fbx *.obj *.dae *.gltf)\0*.fbx; *.obj; *.dae; *.gltf\0");
                if (file)
                {
                    component.Mesh = EGenerator::CreateMesh(*file);
                    component.SetFilePath(*file);
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

        DrawComponent<PointLightComponent>(ICON_ELECTRO_LIGHTBULB_O" PointLight", entity, [](PointLightComponent& component)
        {
            UI::Float("Intensity", &component.Intensity);
            UI::Color3("Color", component.Color);
        });

        DrawComponent<DirectionalLightComponent>(ICON_ELECTRO_SUN_O" DirectionalLight", entity, [](DirectionalLightComponent& component)
        {
            UI::Float("Intensity", &component.Intensity);
            UI::Color3("Color", component.Color);
        });

        DrawComponent<ScriptComponent>(ICON_ELECTRO_CODE" Script", entity, [=](ScriptComponent& component)
        {
            if (UI::ScriptText("Module Name", component.ModuleName, 100.0f, ScriptEngine::ModuleExists(component.ModuleName)))
            {
                if (ScriptEngine::ModuleExists(component.ModuleName))
                    ScriptEngine::InitScriptEntity(entity);
            }
        });
        DrawComponent<RigidBodyComponent>("Rigidbody", entity, [](RigidBodyComponent& rbc)
        {
            const char* rbTypeStrings[] = { "Static", "Dynamic" };
            UI::Dropdown("Rigidbody Type", rbTypeStrings, 2, (int32_t*)&rbc.BodyType);

            if (rbc.BodyType == RigidBodyComponent::Type::Dynamic)
            {
                if (!rbc.IsKinematic)
                {
                    const char* collisionDetectionTypeStrings[] = { "Discrete", "Continious" };
                    UI::Dropdown("Collision Detection", collisionDetectionTypeStrings, 2, (int32_t*)&rbc.CollisionDetectionMode);
                }

                UI::Float("Mass", &rbc.Mass);
                UI::Float("Linear Drag", &rbc.LinearDrag);
                UI::Float("Angular Drag", &rbc.AngularDrag);
                UI::Checkbox("Disable Gravity", &rbc.DisableGravity);
                UI::Checkbox("Is Kinematic", &rbc.IsKinematic);

                if (UI::BeginTreeNode("Constraints", false))
                {
                    ImGui::TextUnformatted("Freeze Position");
                    UI::Checkbox("PositionX", &rbc.LockPositionX);
                    UI::Checkbox("PositionY", &rbc.LockPositionY);
                    UI::Checkbox("PositionZ", &rbc.LockPositionZ);

                    ImGui::TextUnformatted("Freeze Rotation");
                    UI::Checkbox("RotationX", &rbc.LockRotationX);
                    UI::Checkbox("RotationY", &rbc.LockRotationY);
                    UI::Checkbox("RotationZ", &rbc.LockRotationZ);
                    UI::EndTreeNode();
                }
            }
        });
        DrawComponent<BoxColliderComponent>("BoxCollider", entity, [](BoxColliderComponent& bcc)
        {
            if (UI::Float3("Size", bcc.Size))
                bcc.DebugMesh = MeshFactory::CreateCube(bcc.Size);

            UI::Checkbox("Is Trigger", &bcc.IsTrigger);
        });
        DrawComponent<SphereColliderComponent>("SphereCollider", entity, [](SphereColliderComponent& scc)
        {
            if (UI::Float("Radius", &scc.Radius))
                scc.DebugMesh = MeshFactory::CreateSphere(scc.Radius);

            UI::Checkbox("Is Trigger", &scc.IsTrigger);
        });
        DrawComponent<CapsuleColliderComponent>("Capsule Collider", entity, [=](CapsuleColliderComponent& ccc)
        {
            UI::Float("Radius", &ccc.Radius);
            UI::Float("Height", &ccc.Height);
            UI::Checkbox("Is Trigger", &ccc.IsTrigger);
        });
        DrawComponent<MeshColliderComponent>("Mesh Collider", entity, [&](MeshColliderComponent& mcc)
        {
            if (!mcc.CollisionMesh)
            {
                if (entity.HasComponent<MeshComponent>())
                    mcc.CollisionMesh = entity.GetComponent<MeshComponent>().Mesh;
                ImGui::TextColored(ImVec4(0.8f, 0.1f, 0.1f, 1.0f), "Invalid Mesh, Open a mesh in the MeshComponent\nof this entity, or use the OverrideMesh");
            }

            if (mcc.OverrideMesh)
            {
                ImGui::Text("File Path");
                ImGui::SameLine();
                if (mcc.CollisionMesh)
                    ImGui::InputText("##meshfilepath", (char*)mcc.CollisionMesh->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
                else
                    ImGui::InputText("##meshfilepath", (char*)"", 256, ImGuiInputTextFlags_ReadOnly);
                ImGui::SameLine();
                if (ImGui::Button("Open"))
                {
                    auto file = OS::OpenFile("ObjectFile (*.fbx *.obj *.dae)\0*.fbx; *.obj; *.dae\0");
                    if (file)
                    {
                        mcc.CollisionMesh = Ref<Mesh>::Create(*file);
                        if (mcc.IsConvex)
                            PhysXInternal::CreateConvexMesh(mcc, glm::vec3(1.0f), true);
                        else
                            PhysXInternal::CreateTriangleMesh(mcc, glm::vec3(1.0f), true);
                    }
                }
            }

            if (UI::Checkbox("Is Convex", &mcc.IsConvex))
            {
                if (mcc.IsConvex)
                    PhysXInternal::CreateConvexMesh(mcc, glm::vec3(1.0f), true);
                else
                    PhysXInternal::CreateTriangleMesh(mcc, glm::vec3(1.0f), true);
            }

            if (UI::Checkbox("Override Mesh", &mcc.OverrideMesh))
            {
                if (!mcc.OverrideMesh && entity.HasComponent<MeshComponent>())
                {
                    mcc.CollisionMesh = entity.GetComponent<MeshComponent>().Mesh;

                    if (mcc.IsConvex)
                        PhysXInternal::CreateConvexMesh(mcc, glm::vec3(1.0f), true);
                    else
                        PhysXInternal::CreateTriangleMesh(mcc, glm::vec3(1.0f), true);
                }
            }
            UI::Checkbox("Is Trigger", &mcc.IsTrigger);

            if(!mcc.IsConvex)
            {
                if (ImGui::Button("Cook Outline"))
                {
                    Vector<physx::PxShape*> shapes;
                    if (mcc.IsConvex)
                        shapes = PhysXInternal::CreateConvexMesh(mcc, glm::vec3(1.0f), true);
                    else
                        shapes = PhysXInternal::CreateTriangleMesh(mcc, glm::vec3(1.0f), true);

                    PhysXInternal::CookMeshBounds(mcc, shapes);
                }
                UI::ToolTip("Outline's are expensive to cook and might affect the editor FPS.\nThink twice before cooking one!");
                ImGui::SameLine();
                if (ImGui::Button("Destroy Outline"))
                    mcc.ProcessedMeshes.clear();
            }
        });
        DrawComponent<PhysicsMaterialComponent>("PhysicsMaterial", entity, [](PhysicsMaterialComponent& pmc)
        {
            UI::Float("Static Friction", &pmc.StaticFriction, 120.0f);
            UI::Float("Dynamic Friction", &pmc.DynamicFriction, 120.0f);
            UI::Float("Bounciness", &pmc.Bounciness, 120.0f);
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
                if (ImGui::MenuItem("DirectionalLight"))
                {
                    if (!entity.HasComponent<DirectionalLightComponent>())
                        entity.AddComponent<DirectionalLightComponent>();
                    else
                        ELECTRO_WARN("This entity already has DirectionalLight component!");
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Physics"))
            {
                if (ImGui::MenuItem("RigidBody"))
                {
                    if (!entity.HasComponent<RigidBodyComponent>())
                        entity.AddComponent<RigidBodyComponent>();
                    else
                        ELECTRO_WARN("This entity already has RigidBody component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("PhysicsMaterial"))
                {
                    if (!entity.HasComponent<PhysicsMaterialComponent>())
                        entity.AddComponent<PhysicsMaterialComponent>();
                    else
                        ELECTRO_WARN("This entity already has PhysicsMaterial component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("BoxCollider"))
                {
                    if (!entity.HasComponent<BoxColliderComponent>())
                        entity.AddComponent<BoxColliderComponent>();
                    else
                        ELECTRO_WARN("This entity already has BoxCollider component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("SphereCollider"))
                {
                    if (!entity.HasComponent<SphereColliderComponent>())
                        entity.AddComponent<SphereColliderComponent>();
                    else
                        ELECTRO_WARN("This entity already has SphereCollider component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("CapsuleCollider"))
                {
                    if (!entity.HasComponent<CapsuleColliderComponent>())
                        entity.AddComponent<CapsuleColliderComponent>();
                    else
                        ELECTRO_WARN("This entity already has CapsuleCollider component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("MeshCollider"))
                {
                    if (!entity.HasComponent<MeshColliderComponent>())
                        entity.AddComponent<MeshColliderComponent>();
                    else
                        ELECTRO_WARN("This entity already has MeshCollider component!");
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