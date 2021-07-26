//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "CurrentAppPath.hpp"

#if defined(ELECTRO_PLATFORM_WINDOWS)
#include <windows.h>
#include <Shlwapi.h>
#include <io.h> 

#define access _access_s
#endif

#ifdef __APPLE__
#include <libgen.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <unistd.h>
#endif

#ifdef __linux__
#include <limits.h>
#include <libgen.h>
#include <unistd.h>

#if defined(__sun)
#define PROC_SELF_EXE "/proc/self/path/a.out"
#else
#define PROC_SELF_EXE "/proc/self/exe"
#endif

#endif

namespace Electro::CurrentAppPath
{
#if defined(ELECTRO_PLATFORM_WINDOWS)
    String GetExecutablePath()
    {
        char rawPathName[MAX_PATH];
        GetModuleFileNameA(NULL, rawPathName, MAX_PATH);
        return String(rawPathName);
    }
#endif

#ifdef ELECTRO_PLATFORM_LINUX
    String GetExecutablePath()
    {
        char rawPathName[PATH_MAX];
        realpath(PROC_SELF_EXE, rawPathName);
        return  String(rawPathName);
    }
#endif

#ifdef ELECTRO_PLATFORM_MACOS
    String GetExecutablePath()
    {
        char rawPathName[PATH_MAX];
        char realPathName[PATH_MAX];
        Uint rawPathSize = (Uint)sizeof(rawPathName);

        if (!_NSGetExecutablePath(rawPathName, &rawPathSize))
        {
            realpath(rawPathName, realPathName);
        }
        return  String(realPathName);
    }
#endif
}
