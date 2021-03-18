#pragma once
#define E_PLATFORM_WINDOWS

#ifdef E_PLATFORM_WINDOWS
    #ifdef E_BUILD_DLL
        #define ELECTRO_API __declspec(dllexport)
    #else
        #define ELECTRO_API __declspec(dllimport)
    #endif //E_BUILD_DLL
#else
    #error Electro only supports Windows!
#endif //E_PLATFORM_WINDOWS
