//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"

namespace Electro
{
    struct ExporterOptions
    {
        String ExportPath;
        String ApplicationName;
    };

    class RuntimeExporter
    {
    public:
        // Exports the currently active Project
        static void ExportCurrent(const ExporterOptions& options);
    };
}
