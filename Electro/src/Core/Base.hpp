//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <memory>
#include <string>
#include <vector>
#include <deque>
// Platform detection using predefined macros
#ifdef _WIN32
    /* Windows x64/x86 */
    #ifdef _WIN64
        /* Windows x64  */
        #define ELECTRO_PLATFORM_WINDOWS
    #else
        /* Windows x86 */
        #error "x86 Builds are not supported!"
    #endif
#elif defined(__APPLE__) || defined(__MACH__)
    #include <TargetConditionals.h>
    /* TARGET_OS_MAC exists on all the platforms
     * so we must check all of them (in this order)
     * to ensure that we're running on MAC
     * and not some other Apple platform */
    #if TARGET_IPHONE_SIMULATOR == 1
        #error "IOS simulator is not supported!"
    #elif TARGET_OS_IPHONE == 1
        #define ELECTRO_PLATFORM_IOS
        #error "IOS is not supported!"
    #elif TARGET_OS_MAC == 1
        #define ELECTRO_PLATFORM_MACOS
        #error "MacOS is not supported!"
    #else
        #error "Unknown Apple platform!"
    #endif
/* We also have to check __ANDROID__ before __linux__
 * since android is based on the linux kernel
 * it has __linux__ defined */
#elif defined(__ANDROID__)
    #define ELECTRO_PLATFORM_ANDROID
    #error "Android is not supported!"
#elif defined(__linux__)
    #define ELECTRO_PLATFORM_LINUX
    #error "Linux is not supported!"
#else
    /* Unknown compiler/platform */
    #error "Unknown platform!"
#endif // End of platform detection
#ifdef E_DEBUG
    #if defined(ELECTRO_PLATFORM_WINDOWS)
        #define E_DEBUGBREAK() __debugbreak()
    #elif defined(ELECTRO_PLATFORM_LINUX)
        #include<signal.h>
        #define E_DEBUGBREAK() raise(SIGTRAP)
    #else
        #error "ELECTRO doesn't support debugbreak on this platform!"
    #endif
    #define ELECTRO_ENABLE_ASSERTS
#else
    #define E_DEBUGBREAK()
#endif

#ifdef ELECTRO_ENABLE_ASSERTS
    #define E_ASSERT(x, ...) { if(!(x)) { Log::Error("Assertion Failed: {0}", __VA_ARGS__); E_DEBUGBREAK(); } }
    #define E_INTERNAL_ASSERT(x) {  Log::Critical("{0}", x); E_DEBUGBREAK(); }
#else
    #define E_ASSERT(x, ...)
    #define E_INTERNAL_ASSERT(...)
#endif

#define E_BIT(x) (1 << x)
#define ELECTRO_BIND_EVENT_FN(fn) [this](auto&&... args)->decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#define E_FORCE_INLINE __forceinline

namespace Electro
{
    using String = std::string;
    using Uint = uint32_t;
    using byte = uint8_t;

    template<typename T>
    using Vector = std::vector<T>;

    template<typename T>
    using Deque = std::deque<T>;

    using RendererID = void*;

    template<typename T>
    using Scope = std::unique_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args) { return std::make_unique<T>(std::forward<Args>(args)...); }

    template<typename T1, typename T2>
    struct Pair
    {
        T1 Data1;
        T2 Data2;
    };
}
