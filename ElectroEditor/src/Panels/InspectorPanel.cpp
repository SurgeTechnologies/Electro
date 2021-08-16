//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "InspectorPanel.hpp"
#include "Physics/PhysX/PhysXInternal.hpp"
#include "Scripting/ScriptEngine.hpp"

#include "PanelManager.hpp"
#include "SceneHierarchyPanel.hpp"
#include "AssetsPanel.hpp"
#include "AssetEditors/MaterialEditor.hpp"
#include "AssetEditors/PhysicsMaterialEditor.hpp"
#include "UIUtils/UIUtils.hpp"
#include "UIMacros.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

namespace Electro
{
    void InspectorPanel::OnInit(void* data)
    {
        mSceneHierarchy = PanelManager::GetPanel<SceneHierarchyPanel>(HIERARCHY_TITLE);
        mAsssetsPanel = PanelManager::GetPanel<AssetsPanel>(ASSETS_TITLE);

        mAssetEditors[AssetType::MATERIAL] = CreateScope<MaterialEditor>();
        mAssetEditors[AssetType::PHYSICS_MATERIAL] = CreateScope<PhysicsMaterialEditor>();

        E_ASSERT(mSceneHierarchy != nullptr, "Cannot find SceneHierarchyPanel!");
    }

    void InspectorPanel::OnImGuiRender(bool* show)
    {
        // Inspector
        ImGui::Begin(INSPECTOR_TITLE, show);

        if (mSceneHierarchy->mSelectionContext)
            DrawComponents(mSceneHierarchy->mSelectionContext);
        else if (mAsssetsPanel->GetSelectedEntry().Extension == ".emat")
            mAssetEditors[AssetType::MATERIAL]->Render();
        else if (mAsssetsPanel->GetSelectedEntry().Extension == ".epmat")
            mAssetEditors[AssetType::PHYSICS_MATERIAL]->Render();

        ImGui::End();
    }

    template<typename T, typename UIFunction>
    static void DrawComponent(const String& name, Entity entity, UIFunction uiFunction)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
        if (entity.HasComponent<T>())
        {
            ImGui::PushID(reinterpret_cast<void*>(typeid(T).hash_code()));
            auto& component = entity.GetComponent<T>();
            const ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
            const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

            const bool open = ImGui::TreeNodeEx("##dummyID", treeNodeFlags, name.c_str());
            ImGui::PopStyleVar();

            ImGui::SameLine(contentRegionAvailable.x - 35.0f - lineHeight * 0.5f);
            UI::ToggleButton("Toggle", &component.Active);

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

    void InspectorPanel::DrawComponents(Entity entity)
    {
        auto ID = entity.GetComponent<IDComponent>().ID;

        if (entity.HasComponent<TagComponent>())
        {
            ImGui::PushID(static_cast<int>(ID));
            ImGui::PushItemWidth(-1);
            ImGui::InputText("##ID", &entity.GetComponent<TagComponent>().Tag);
            ImGui::PopItemWidth();
            ImGui::PopID();
        }

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
                const char* currentProjectionTypeString = projectionTypeStrings[static_cast<int>(camera.GetProjectionType())];
                ImGui::Columns(2);
                ImGui::Text("Projection");
                ImGui::SetColumnWidth(0, 160.0f);
                ImGui::NextColumn();
                ImGui::PushItemWidth(-1);
                if (ImGui::BeginCombo("##Projection", currentProjectionTypeString))
                {
                    for (int i = 0; i < 2; i++)
                    {
                        const bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                        if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
                        {
                            currentProjectionTypeString = projectionTypeStrings[i];
                            camera.SetProjectionType(static_cast<SceneCamera::ProjectionType>(i));
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

        DrawComponent<MeshComponent>(ICON_ELECTRO_CUBE" Mesh", entity, [](MeshComponent& component)
            {
                if (component.Mesh)
                    ImGui::InputTextWithHint("##mesh", component.Mesh->GetPath().c_str(), "", sizeof(""), ImGuiInputTextFlags_ReadOnly);
                else
                {
                    ImGui::InputTextWithHint("##mesh", "", "", sizeof(""), ImGuiInputTextFlags_ReadOnly);
                    UI::ToolTip("You can Drag and Drop Mesh(3D file like *.obj,\n*.fbx etc.) from " ASSETS_TITLE " here!");
                }

                if (const ImGuiPayload* dropData = UI::DragAndDropTarget(MESH_DND_ID); dropData)
                    component.Mesh = Mesh::Create(*static_cast<String*>(dropData->Data));

                if (component.Mesh)
                {
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Remove"))
                        component.Mesh.Reset();

                    UI::Checkbox("Cast Shadows", &component.CastShadows);

                    int index = 0;
                    if (ImGui::BeginTable("MeshMaterials", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders))
                    {
                        for (Ref<Material>& mat : component.Mesh->GetMaterials())
                        {
                            if (!mat)
                                continue;

                            ImGui::PushID(index);
                            ImGui::TableNextColumn();

                            if (IsAssetHandleValid(mat->GetHandle())) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.8f, 0.1f, 1.0f));
                            ImGui::Text("Material %i", index);
                            if (IsAssetHandleValid(mat->GetHandle())) ImGui::PopStyleColor();

                            ImGui::TableNextColumn();

                            mat ? ImGui::TextUnformatted(mat->GetName().c_str()) : ImGui::TextUnformatted("Default");

                            const ImGuiPayload* droppedData = UI::DragAndDropTarget(MATERIAL_DND_ID);
                            if (droppedData)
                            {
                                AssetHandle handle = *(AssetHandle*)droppedData->Data;
                                if (IsAssetHandleValid(handle))
                                {
                                    mat.Reset();
                                    mat = AssetManager::GetAsset<Material>(handle);
                                }
                            }

                            ImGui::PopID();
                            index++;
                        }
                        ImGui::EndTable();
                    }
                }
            });

        DrawComponent<PointLightComponent>(ICON_ELECTRO_LIGHTBULB_O" PointLight", entity, [](PointLightComponent& component)
            {
                if (ImGui::BeginTable("pointLight", 2))
                {
                    ImGui::TableSetupColumn("##plightCol", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 100.0f);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Intensity");
                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(-1);
                    ImGui::DragFloat("##intensity", &component.Intensity);
                    ImGui::PopItemWidth();

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Radius");
                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(-1);
                    ImGui::DragFloat("##radius", &component.Radius);
                    ImGui::PopItemWidth();

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Color");
                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(-1);
                    ImGui::ColorEdit3("##color", glm::value_ptr(component.Color));
                    ImGui::PopItemWidth();

                    ImGui::EndTable();
                }
            });

        DrawComponent<DirectionalLightComponent>(ICON_ELECTRO_SUN_O" DirectionalLight", entity, [](DirectionalLightComponent& component)
            {
                if (ImGui::BeginTable("directionalLight", 2))
                {
                    ImGui::TableSetupColumn("##dlightCol", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 100.0f);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Intensity");
                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(-1);
                    ImGui::DragFloat("##intensity", &component.Intensity);
                    ImGui::PopItemWidth();

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Color");
                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(-1);
                    ImGui::ColorEdit3("##color", glm::value_ptr(component.Color));
                    ImGui::PopItemWidth();

                    ImGui::EndTable();
                }
            });

        DrawComponent<ScriptComponent>(ICON_ELECTRO_CODE" Script", entity, [=](ScriptComponent& component) mutable
            {
                String oldName = component.ModuleName;
                if (UI::ScriptText("Module Name", component.ModuleName, 100.0f, ScriptEngine::ModuleExists(component.ModuleName)))
                {
                    if (ScriptEngine::ModuleExists(oldName))
                        ScriptEngine::ShutdownScriptEntity(entity, oldName);

                    if (ScriptEngine::ModuleExists(component.ModuleName))
                        ScriptEngine::InitScriptEntity(entity);
                }

                if (ScriptEngine::ModuleExists(component.ModuleName))
                {
                    EntityInstanceData& entityInstanceData = ScriptEngine::GetEntityInstanceData(entity.GetSceneUUID(), ID);
                    ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
                    if (moduleFieldMap.find(component.ModuleName) != moduleFieldMap.end())
                    {
                        std::unordered_map<String, PublicField>& publicFields = moduleFieldMap.at(component.ModuleName);
                        for (auto& [name, field] : publicFields)
                        {
                            bool isRuntime = mSceneHierarchy->mContext->IsPlaying() && field.IsRuntimeAvailable();
                            switch (field.mType)
                            {
                            case FieldType::INT:
                            {
                                int value = isRuntime ? field.GetRuntimeValue<int>() : field.GetStoredValue<int>();
                                if (UI::Int(field.mName.c_str(), &value))
                                {
                                    if (isRuntime)
                                        field.SetRuntimeValue(value);
                                    else
                                        field.SetStoredValue(value);
                                }
                                break;
                            }
                            case FieldType::FLOAT:
                            {
                                float value = isRuntime ? field.GetRuntimeValue<float>() : field.GetStoredValue<float>();
                                if (UI::Float(field.mName.c_str(), &value))
                                {
                                    if (isRuntime)
                                        field.SetRuntimeValue(value);
                                    else
                                        field.SetStoredValue(value);
                                }
                                break;
                            }
                            case FieldType::VEC2:
                            {
                                glm::vec2 value = isRuntime ? field.GetRuntimeValue<glm::vec2>() : field.GetStoredValue<glm::vec2>();
                                if (UI::Float2(field.mName.c_str(), value))
                                {
                                    if (isRuntime)
                                        field.SetRuntimeValue(value);
                                    else
                                        field.SetStoredValue(value);
                                }
                                break;
                            }
                            case FieldType::VEC3:
                            {
                                glm::vec3 value = isRuntime ? field.GetRuntimeValue<glm::vec3>() : field.GetStoredValue<glm::vec3>();
                                if (UI::Float3(field.mName.c_str(), value))
                                {
                                    if (isRuntime)
                                        field.SetRuntimeValue(value);
                                    else
                                        field.SetStoredValue(value);
                                }
                                break;
                            }
                            case FieldType::VEC4:
                            {
                                glm::vec4 value = isRuntime ? field.GetRuntimeValue<glm::vec4>() : field.GetStoredValue<glm::vec4>();
                                if (UI::Float4(field.mName.c_str(), value))
                                {
                                    if (isRuntime)
                                        field.SetRuntimeValue(value);
                                    else
                                        field.SetStoredValue(value);
                                }
                                break;
                            }
                            }
                        }
                    }
                }
            });
        DrawComponent<RigidBodyComponent>("Rigidbody", entity, [](RigidBodyComponent& rbc)
            {
                const char* rbTypeStrings[] = { "Static", "Dynamic" };

                UI::Dropdown("Rigidbody Type", rbTypeStrings, 2, reinterpret_cast<int32_t*>(&rbc.BodyType));

                if (rbc.BodyType == RigidBodyComponent::Type::DYNAMIC)
                {
                    if (!rbc.IsKinematic)
                    {
                        const char* collisionDetectionTypeStrings[] = { "Discrete", "Continuous" };
                        UI::Dropdown("Collision Detection", collisionDetectionTypeStrings, 2, reinterpret_cast<int32_t*>(&rbc.CollisionDetectionMode));
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
                if (ImGui::BeginTable("pmat", 2, ImGuiTableFlags_BordersInnerV))
                {
                    // Physics Material + Drop
                    ImGui::TableSetupColumn("##pmatCol", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 100.0f);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Physics Material");
                    ImGui::TableNextColumn();
                    if (!bcc.PhysicsMaterial)
                        ImGui::TextUnformatted("Global Physics Material");
                    else
                    {
                        ImGui::TextUnformatted(AssetManager::GetMetadata(bcc.PhysicsMaterial->GetHandle()).Path.stem().string().c_str());
                        ImGui::SameLine();
                        if (ImGui::Button(ICON_ELECTRO_TRASH))
                            bcc.PhysicsMaterial.Reset();
                    }
                    const ImGuiPayload* droppedData = UI::DragAndDropTarget(PHYSICS_MAT_DND_ID);
                    if (droppedData)
                    {
                        AssetHandle handle = *(AssetHandle*)droppedData->Data;
                        if (IsAssetHandleValid(handle))
                            bcc.PhysicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(handle);
                    }

                    // Size
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Size");
                    ImGui::TableNextColumn();
                    if (ImGui::DragFloat3("##size", glm::value_ptr(bcc.Size)))
                        bcc.DebugMesh = MeshFactory::CreateCube(bcc.Size);

                    // Offset
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Offset");
                    ImGui::TableNextColumn();
                    ImGui::DragFloat3("##offset", glm::value_ptr(bcc.Offset));

                    // Is Trigger
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Is Trigger");
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##isTrigger", &bcc.IsTrigger);

                    // Show Bounds
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Show Bounds");
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##show Bounds", &bcc.ShowColliderBounds);

                    ImGui::EndTable();
                }
            });
        DrawComponent<SphereColliderComponent>("SphereCollider", entity, [](SphereColliderComponent& scc)
            {
                if (ImGui::BeginTable("pmat", 2, ImGuiTableFlags_BordersInnerV))
                {
                    // Physics Material + Drop
                    ImGui::TableSetupColumn("##pmatCol", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 100.0f);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Physics Material");
                    ImGui::TableNextColumn();
                    if (!scc.PhysicsMaterial)
                        ImGui::TextUnformatted("Global Physics Material");
                    else
                    {
                        ImGui::TextUnformatted(AssetManager::GetMetadata(scc.PhysicsMaterial->GetHandle()).Path.stem().string().c_str());
                        ImGui::SameLine();
                        if (ImGui::Button(ICON_ELECTRO_TRASH))
                            scc.PhysicsMaterial.Reset();
                    }
                    const ImGuiPayload* droppedData = UI::DragAndDropTarget(PHYSICS_MAT_DND_ID);
                    if (droppedData)
                    {
                        AssetHandle handle = *(AssetHandle*)droppedData->Data;
                        if (IsAssetHandleValid(handle))
                            scc.PhysicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(handle);
                    }

                    // Radius
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Radius");
                    ImGui::TableNextColumn();
                    if (ImGui::DragFloat("##radius", &scc.Radius))
                        scc.DebugMesh = MeshFactory::CreateSphere(scc.Radius);

                    // Is Trigger
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Is Trigger");
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##isTrigger", &scc.IsTrigger);

                    // Show Bounds
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Show Bounds");
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##show Bounds", &scc.ShowColliderBounds);

                    ImGui::EndTable();
                }
            });
        DrawComponent<CapsuleColliderComponent>("Capsule Collider", entity, [=](CapsuleColliderComponent& ccc)
            {
                if (ImGui::BeginTable("pmat", 2, ImGuiTableFlags_BordersInnerV))
                {
                    // Physics Material + Drop
                    ImGui::TableSetupColumn("##pmatCol", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 100.0f);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Physics Material");
                    ImGui::TableNextColumn();
                    if (!ccc.PhysicsMaterial)
                        ImGui::TextUnformatted("Global Physics Material");
                    else
                    {
                        ImGui::TextUnformatted(AssetManager::GetMetadata(ccc.PhysicsMaterial->GetHandle()).Path.stem().string().c_str());
                        ImGui::SameLine();
                        if (ImGui::Button(ICON_ELECTRO_TRASH))
                            ccc.PhysicsMaterial.Reset();
                    }
                    const ImGuiPayload* droppedData = UI::DragAndDropTarget(PHYSICS_MAT_DND_ID);
                    if (droppedData)
                    {
                        AssetHandle handle = *(AssetHandle*)droppedData->Data;
                        if (IsAssetHandleValid(handle))
                            ccc.PhysicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(handle);
                    }

                    // Height
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Height");
                    ImGui::TableNextColumn();
                    if (ImGui::DragFloat("##height", &ccc.Height))
                        ccc.DebugMesh = MeshFactory::CreateCapsule(ccc.Radius, ccc.Height);

                    // Radius
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Radius");
                    ImGui::TableNextColumn();
                    if (ImGui::DragFloat("##radius", &ccc.Radius))
                        ccc.DebugMesh = MeshFactory::CreateCapsule(ccc.Radius, ccc.Height);

                    // Is Trigger
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Is Trigger");
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##isTrigger", &ccc.IsTrigger);

                    // Show Bounds
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Show Bounds");
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##show Bounds", &ccc.ShowColliderBounds);

                    ImGui::EndTable();
                }
            });
        DrawComponent<MeshColliderComponent>("Mesh Collider", entity, [&](MeshColliderComponent& mcc)
            {
                if (!mcc.CollisionMesh)
                {
                    if (entity.HasComponent<MeshComponent>())
                        mcc.CollisionMesh = entity.GetComponent<MeshComponent>().Mesh;
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
                    ImGui::TextWrapped("Invalid Mesh, Open a mesh in the MeshComponent of this entity, or use the OverrideMesh");
                    ImGui::PopStyleColor();
                }

                if (mcc.OverrideMesh)
                {
                    ImGui::TextUnformatted("Path");
                    ImGui::SameLine();
                    if (mcc.CollisionMesh)
                        ImGui::InputText("##meshfilepath", const_cast<char*>(mcc.CollisionMesh->GetPath().c_str()), 256, ImGuiInputTextFlags_ReadOnly);
                    else
                        ImGui::InputText("##meshfilepath", "[Null]", 256, ImGuiInputTextFlags_ReadOnly);

                    if (const ImGuiPayload* dropData = UI::DragAndDropTarget(MESH_DND_ID); dropData)
                    {
                        mcc.CollisionMesh = Mesh::Create(*static_cast<String*>(dropData->Data));
                        if (mcc.CollisionMesh)
                        {
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
                UI::Checkbox("ShowBounds", &mcc.ShowColliderBounds);
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                    ImGui::TextUnformatted("MeshCollider bounds are usually expensive to draw and can heavily affect the editor FPS!\n"
                        "You will probably want to see them once and uncheck this when done!");
                    ImGui::PopStyleColor();
                    ImGui::EndTooltip();
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
                        Log::Warn("This entity already has Camera component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Mesh"))
                {
                    if (!entity.HasComponent<MeshComponent>())
                        entity.AddComponent<MeshComponent>();
                    else
                        Log::Warn("This entity already has Mesh component!");
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
                        Log::Warn("This entity already has PointLight component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("DirectionalLight"))
                {
                    if (!entity.HasComponent<DirectionalLightComponent>())
                        entity.AddComponent<DirectionalLightComponent>();
                    else
                        Log::Warn("This entity already has DirectionalLight component!");
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
                        Log::Warn("This entity already has RigidBody component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("BoxCollider"))
                {
                    if (!entity.HasComponent<BoxColliderComponent>())
                        entity.AddComponent<BoxColliderComponent>();
                    else
                        Log::Warn("This entity already has BoxCollider component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("SphereCollider"))
                {
                    if (!entity.HasComponent<SphereColliderComponent>())
                        entity.AddComponent<SphereColliderComponent>();
                    else
                        Log::Warn("This entity already has SphereCollider component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("CapsuleCollider"))
                {
                    if (!entity.HasComponent<CapsuleColliderComponent>())
                        entity.AddComponent<CapsuleColliderComponent>();
                    else
                        Log::Warn("This entity already has CapsuleCollider component!");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("MeshCollider"))
                {
                    if (!entity.HasComponent<MeshColliderComponent>())
                        entity.AddComponent<MeshColliderComponent>();
                    else
                        Log::Warn("This entity already has MeshCollider component!");
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Script"))
            {
                if (!entity.HasComponent<ScriptComponent>())
                    entity.AddComponent<ScriptComponent>();
                else
                    Log::Warn("This entity already has Script component!");
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}
