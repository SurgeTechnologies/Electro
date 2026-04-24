# Copyright(c) 2021 - Electro Team - All rights reserved
cmake_minimum_required(VERSION 3.16)

project(Electro)
set(CMAKE_CXX_STANDARD 17)

# Detect desktop vs mobile build targets
if(ANDROID OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(ELECTRO_MOBILE TRUE)
else()
    set(ELECTRO_MOBILE FALSE)
endif()

if(ELECTRO_MOBILE)
    message(STATUS "Electro: configuring for mobile target (${CMAKE_SYSTEM_NAME})")
else()
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ElectroEditor)
endif()

#Prevents in source build
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

if(MSVC)
    # MSVC-specific flags (Windows desktop only)
    set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /std:c++17 /W4 /arch:AVX2 /fp:fast /MP /DNOMINMAX")
    # Ignore warnings about missing pdb
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    /ignore:4099")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")
    # Static runtime linkage
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   /MTd")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    # Clang / Apple Clang (Android NDK, iOS, or desktop Clang)
    set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -Wall -Wextra -ffast-math")
    if(NOT ANDROID AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
        # AVX2/FMA only makes sense on x86-64 desktop; ARM has its own SIMD
        set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -mavx2 -mfma -m64")
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -Wall -Wextra -ffast-math")
    if(NOT ANDROID)
        set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -mavx2 -mfma -m64")
    endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(Electro/vendor/imgui)
add_subdirectory(Electro/vendor/Yaml)
add_subdirectory(Electro/vendor/FMT)
add_subdirectory(Electro/vendor/SPIRV-Cross)
add_subdirectory(Electro/vendor/Meshoptimizer)

add_subdirectory(Electro)

if(NOT ELECTRO_MOBILE)
    # Editor, scripting, and runtime launcher are desktop-only targets
    add_subdirectory(ElectroEditor)
    add_subdirectory(ElectroRuntime)
    add_subdirectory(ElectroScript-Core)
endif()
