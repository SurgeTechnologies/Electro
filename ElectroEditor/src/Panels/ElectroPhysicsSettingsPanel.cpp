//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroPhysicsSettingsPanel.hpp"
#include "Physics/ElectroPhysicsEngine.hpp"
#include "Physics/ElectroPhysicsLayer.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace Electro
{
    void PhysicsSettingsWindow::OnImGuiRender(bool* show)
    {
        if (!show)
            return;

        ImGui::Begin("Physics Settings", show);
        RenderWorldSettings();
        ImGui::End();
    }

    void PhysicsSettingsWindow::RenderWorldSettings()
    {
        PhysicsSettings& settings = PhysicsEngine::GetSettings();
        UI::DrawFloatControl("Fixed Timestep", &settings.FixedTimestep);
        UI::DrawFloatControl("Gravity", &settings.Gravity.y);

        static const char* broadphaseTypeStrings[] = { "Sweep and Prune", "MultiBox Pruning", "Automatic Box Pruning" };
        UI::DrawDropdown("Broadphase Type", broadphaseTypeStrings, 3, (int*)&settings.BroadphaseAlgorithm);
        if (settings.BroadphaseAlgorithm != BroadphaseType::AutomaticBoxPrune)
        {
            UI::DrawFloat3Control("World Bounds (Min)", settings.WorldBoundsMin, 120);
            UI::DrawFloat3Control("World Bounds (Max)", settings.WorldBoundsMax, 120);
            UI::DrawSlider("Grid Subdivisions", (int&)settings.WorldBoundsSubdivisions, 1, 10000);
        }

        static const char* frictionTypeStrings[] = { "Patch", "One Directional", "Two Directional" };
        UI::DrawDropdown("Friction Model", frictionTypeStrings, 3, (int*)&settings.FrictionModel);

        UI::DrawSlider("Solver Iterations", (int&)settings.SolverIterations, 1, 255);
        ImGui::PushID("Solver Iterations");
        ImGui::SameLine();
        if(ImGui::Button("Reset"))
            settings.SolverIterations = 6;
        ImGui::PopID();

        UI::DrawSlider("Solver Velocity Iterations", (int&)settings.SolverVelocityIterations, 1, 255);
        ImGui::PushID("Solver Velocity Iterations");
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
            settings.SolverVelocityIterations = 1;
        ImGui::PopID();
    }
}