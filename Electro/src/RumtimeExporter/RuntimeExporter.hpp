//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"

namespace Electro
{
    class RuntimeExporter
    {
    public:
        // Exports the currently active Project
        static void ExportCurrent(const String& path);
    };
}
