//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <memory>
#include <string>
#include <vector>

#ifdef E_DEBUG
    #if defined(ELECTRO_WINDOWS)
        #define E_DEBUGBREAK() __debugbreak()
    #elif defined(ELECTRO_LINUX)
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
    #define E_ASSERT(x, ...) { if(!(x)) { ELECTRO_ERROR("Assertion Failed: %s", __VA_ARGS__); E_DEBUGBREAK(); } }
    #define E_INTERNAL_ASSERT(x) {  ELECTRO_CRITICAL(x); E_DEBUGBREAK(); }
#else
    #define E_ASSERT(x, ...)
    #define E_INTERNAL_ASSERT(...)
#endif

#define E_BIT(x) (1 << x)
#define ELECTRO_BIND_EVENT_FN(fn) [this](auto&&... args)->decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#define E_FORCE_INLINE __forceinline
#define  E_NODISCARD [[nodiscard]]

namespace Electro
{
    using String = std::string;
    using Uint = uint32_t;
    using byte = uint8_t;

    template<typename T>
    using Vector = std::vector<T>;
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
