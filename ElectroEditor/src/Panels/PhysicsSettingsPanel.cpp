//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "PhysicsSettingsPanel.hpp"
#include "Asset/AssetManager.hpp"
#include "Physics/PhysicsEngine.hpp"
#include "UIUtils/UIUtils.hpp"
#include "UIMacros.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace Electro
{
    void PhysicsSettingsPanel::Init(void* data)
    {
        auto tex = AssetManager::Get<Texture2D>("physx.png");
        mPhysXTextureID = tex->GetRendererID();
        mTextureDimensions[0] = tex->GetWidth();
        mTextureDimensions[1] = tex->GetHeight();
    }

    void PhysicsSettingsPanel::OnImGuiRender(bool* show)
    {
        if (!show)
            return;

        ImGui::Begin(PHYSICS_SETTINGS_TITLE, show);
        PhysicsSettings& settings = PhysicsEngine::GetSettings();
        UI::Float("Fixed Timestep", &settings.FixedTimestep);
        ImGui::SameLine();
        if (ImGui::Button("Reset##FixedTimestep"))
            settings.FixedTimestep = 0.02f;

        UI::Float3("Gravity", settings.Gravity);
        static const char* broadphaseTypeStrings[] = { "Sweep and Prune", "MultiBox Pruning", "Automatic Box Pruning" };
        UI::Dropdown("Broadphase Type", broadphaseTypeStrings, 3, (int*)&settings.BroadphaseAlgorithm);
        if (settings.BroadphaseAlgorithm != BroadphaseType::AutomaticBoxPrune)
        {
            UI::Float3("World Bounds (Min)", settings.WorldBoundsMin, 120);
            UI::Float3("World Bounds (Max)", settings.WorldBoundsMax, 120);
            UI::SliderInt("Grid Subdivisions", (int&)settings.WorldBoundsSubdivisions, 1, 10000);
        }

        static const char* frictionTypeStrings[] = { "Patch", "One Directional", "Two Directional" };
        UI::Dropdown("Friction Model", frictionTypeStrings, 3, (int*)&settings.FrictionModel);

        UI::SliderInt("Solver Iterations", (int&)settings.SolverIterations, 1, 255);
        ImGui::SameLine();
        if (ImGui::Button("Reset##SolverIterations"))
            settings.SolverIterations = 6;

        UI::SliderInt("Solver Velocity Iterations", (int&)settings.SolverVelocityIterations, 1, 255);
        ImGui::SameLine();
        if (ImGui::Button("Reset##SVI"))
            settings.SolverVelocityIterations = 1;

        if (ImGui::TreeNodeEx("Configure GlobalPhysicsMaterial"))
        {
            auto& mat = PhysicsEngine::GetGlobalPhysicsMaterial();
            if (ImGui::Button("Reset##Material"))
            {
                mat->mStaticFriction = 1.0f;
                mat->mDynamicFriction = 1.0f;
                mat->mBounciness = 0.0f;
            }

            UI::Float("Static Friction", &mat->mStaticFriction);
            UI::Float("Dynamic Friction", &mat->mDynamicFriction);
            UI::Float("Bounciness", &mat->mBounciness);
            ImGui::TreePop();
        }

        UI::Image(mPhysXTextureID, { mTextureDimensions[0], mTextureDimensions[1] });
        ImGui::End();
    }
}
