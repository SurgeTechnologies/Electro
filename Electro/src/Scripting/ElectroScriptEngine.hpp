//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once

namespace Electro
{
    class ScriptEngine
    {
    public:
        static void Init(const char* assemblyPath);
        static void Shutdown();
        static void LoadElectroRuntimeAssembly(const char* path);
    };
}