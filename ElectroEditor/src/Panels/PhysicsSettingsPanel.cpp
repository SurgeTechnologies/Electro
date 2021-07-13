//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "PhysicsSettingsPanel.hpp"
#include "Asset/AssetManager.hpp"
#include "Physics/PhysicsEngine.hpp"
#include "UIUtils/UIUtils.hpp"
#include "UIMacros.hpp"
#include <imgui.h>
#include <imgui_internal.h>

namespace Electro
{
    void PhysicsSettingsPanel::OnImGuiRender(bool* show)
    {
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

        if (ImGui::TreeNode("Configure GlobalPhysicsMaterial"))
        {
            auto& mat = PhysicsEngine::GetGlobalPhysicsMaterial();
            if (ImGui::Button("Reset##Material"))
            {
                mat->SetStaticFriction(0.3f);
                mat->SetDynamicFriction(0.3f);
                mat->SetBounciness(0.4f);
            }

            UI::Float("Static Friction", &mat->GetStaticFriction());
            UI::Float("Dynamic Friction", &mat->GetDynamicFriction());
            UI::Float("Bounciness", &mat->GetBounciness());
            ImGui::TreePop();
        }

        ImGui::End();
    }
}
