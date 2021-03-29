//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Electro::Scripting
{
    void InitMono(MonoDomain* domain);
    void ShutdownMono(MonoDomain* domain);
    MonoAssembly* LoadAssembly(const char* path);
    MonoImage* GetAssemblyImage(MonoAssembly* assembly);
}