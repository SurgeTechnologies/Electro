//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroPhysicsSettingsPanel.hpp"
#include "Core/ElectroVault.hpp"
#include "Physics/ElectroPhysicsEngine.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace Electro
{
    void PhysicsSettingsPanel::Init()
    {
        auto tex = Vault::Get<Texture2D>("physx.png");
        mPhysXTextureID = tex->GetRendererID();
        mTextureDimensions[0] = tex->GetWidth();
        mTextureDimensions[1] = tex->GetHeight();
    }

    void PhysicsSettingsPanel::OnImGuiRender(bool* show)
    {
        if (!show)
            return;

        ImGui::Begin("Physics", show);
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
            UI::Slider("Grid Subdivisions", (int&)settings.WorldBoundsSubdivisions, 1, 10000);
        }

        static const char* frictionTypeStrings[] = { "Patch", "One Directional", "Two Directional" };
        UI::Dropdown("Friction Model", frictionTypeStrings, 3, (int*)&settings.FrictionModel);

        UI::Slider("Solver Iterations", (int&)settings.SolverIterations, 1, 255);
        ImGui::SameLine();
        if (ImGui::Button("Reset##SolverIterations"))
            settings.SolverIterations = 6;

        UI::Slider("Solver Velocity Iterations", (int&)settings.SolverVelocityIterations, 1, 255);
        ImGui::SameLine();
        if (ImGui::Button("Reset##SVI"))
            settings.SolverVelocityIterations = 1;

        if (ImGui::TreeNodeEx("Configure GlobalPhysicsMaterial"))
        {
            auto& mat = PhysicsEngine::GetGlobalPhysicsMaterial();
            if (ImGui::Button("Reset##Material"))
            {
                mat.StaticFriction = 1.0f;
                mat.DynamicFriction = 1.0f;
                mat.Bounciness = 0.0f;
            }

            UI::Float("Static Friction", &mat.StaticFriction);
            UI::Float("Dynamic Friction", &mat.DynamicFriction);
            UI::Float("Bounciness", &mat.Bounciness);
            ImGui::TreePop();
        }

        UI::Image(mPhysXTextureID, { mTextureDimensions[0], mTextureDimensions[1] });
        ImGui::End();
    }
}