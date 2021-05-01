//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "ElectroModule.hpp"
#include <vector>

namespace Electro
{
    class ModuleManager
    {
    public:
        ModuleManager() = default;
        ~ModuleManager();

        void PushModule(Module* module);
        void PushOverlay(Module* overlay);
        void PopModule(Module* module);
        void PopOverlay(Module* overlay);

        Vector<Module*>::iterator begin() { return mModules.begin(); }
        Vector<Module*>::iterator end() { return mModules.end(); }
    private:
        Vector<Module*> mModules;
        Uint mModuleInsertIndex = 0;
    };
}