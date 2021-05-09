//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ModuleManager.hpp"

namespace Electro
{
    ModuleManager::~ModuleManager()
    {
        for (Module* module : mModules)
        {
            module->Shutdown();
            delete module;
        }
    }

    void ModuleManager::PushModule(Module* module)
    {
        mModules.emplace(mModules.begin() + mModuleInsertIndex, module);
        mModuleInsertIndex++;
    }

    void ModuleManager::PushOverlay(Module* overlay)
    {
        mModules.push_back(overlay);
    }

    void ModuleManager::PopModule(Module* module)
    {
        auto it = std::find(mModules.begin(), mModules.end(), module);
        if (it != mModules.end())
        {
            mModules.erase(it);
            mModuleInsertIndex--;
            module->Shutdown();
        }
    }

    void ModuleManager::PopOverlay(Module* overlay)
    {
        auto it = std::find(mModules.begin(), mModules.end(), overlay);
        if (it != mModules.end())
        {
            mModules.erase(it);
            overlay->Shutdown();
        }
    }
}
