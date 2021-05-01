//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#define ELECTRO_ENTRYPOINT
#include <Electro.hpp>
#include "ElectroEditorModule.hpp"

namespace Electro
{
    class ElectroEditor : public Application
    {
    public:
        ElectroEditor()
            :Application("Electro Editor")
        {
            PushModule(new EditorModule());
        }

        ~ElectroEditor()
        {
        }
    };

    Application* CreateApplication()
    {
        return new ElectroEditor();
    }
}