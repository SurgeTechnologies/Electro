# Copyright(c) 2021 - Electro Team - All rights reserved
include(${CMAKE_SOURCE_DIR}/scripts/CMakeUtils.cmake)

set(INCLUDE_DIRS
    src
    ${CMAKE_SOURCE_DIR}/ElectroEditor/src
    vendor/imgui
    vendor/stb_image
    vendor/assimp/include
    vendor/glm
    vendor/entt/include
    vendor/Yaml/include
    vendor/FMT/include
    vendor/ElectroMono/include
    vendor/PhysX/include
    vendor/PhysX/include/PhysX #For internal PhsysX usage
    vendor/SPIRV-Cross/include
    vendor/Meshoptimizer/include
)

# ── Platform source selection ─────────────────────────────────────────────────
# Collect all engine source files, then strip out platform-specific directories
# that don't belong to the current build target.
file(GLOB_RECURSE ALL_SOURCE_FILES
    src/*.cpp
    src/*.hpp
    src/*.mm          # Objective-C++ (iOS)
    vendor/stb_image/stb_image.h
    vendor/stb_image/stb_image.cpp
)

# Always remove ALL platform directories first, then add the right one back.
set(PLATFORM_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Windows"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/DX11"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Android"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/iOS"
)
foreach(DIR ${PLATFORM_DIRS})
    foreach(FILE ${ALL_SOURCE_FILES})
        string(FIND "${FILE}" "${DIR}" DIR_POS)
        if(NOT DIR_POS EQUAL -1)
            list(REMOVE_ITEM ALL_SOURCE_FILES "${FILE}")
        endif()
    endforeach()
endforeach()

if(ANDROID)
    file(GLOB_RECURSE PLATFORM_SOURCE_FILES
        src/Platform/Android/*.cpp
        src/Platform/Android/*.hpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    file(GLOB_RECURSE PLATFORM_SOURCE_FILES
        src/Platform/iOS/*.mm
        src/Platform/iOS/*.hpp
    )
else()
    # Windows (or any future desktop platform) – include Win32 + DX11
    file(GLOB_RECURSE PLATFORM_SOURCE_FILES
        src/Platform/Windows/*.cpp
        src/Platform/Windows/*.hpp
        src/Platform/DX11/*.cpp
        src/Platform/DX11/*.hpp
    )
endif()

set(SOURCE_FILES ${ALL_SOURCE_FILES} ${PLATFORM_SOURCE_FILES})

# ── Library links ─────────────────────────────────────────────────────────────
set(LIB_LINKS
    Yaml
    FMT
    SPIRV-Cross
    Meshoptimizer
    imgui
)

if(ANDROID)
    list(APPEND LIB_LINKS android log EGL GLESv3)
    # Assimp, Mono, and PhysX require Android ARM prebuilts (not bundled yet).
    # Add them here once the arm64-v8a static libraries are placed under vendor/.
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    # iOS frameworks
    find_library(UIKIT_LIB UIKit REQUIRED)
    find_library(METAL_LIB Metal REQUIRED)
    find_library(QUARTZCORE_LIB QuartzCore REQUIRED)
    find_library(FOUNDATION_LIB Foundation REQUIRED)
    list(APPEND LIB_LINKS
        ${UIKIT_LIB}
        ${METAL_LIB}
        ${QUARTZCORE_LIB}
        ${FOUNDATION_LIB}
    )
    # Assimp, Mono (AOT only), and PhysX require iOS ARM prebuilts (not bundled yet).
else()
    # Windows desktop
    list(APPEND LIB_LINKS
        opengl32
        ${CMAKE_SOURCE_DIR}/Electro/vendor/assimp/lib/assimp-vc142-mt.lib
        ${CMAKE_SOURCE_DIR}/Electro/vendor/ElectroMono/lib/mono-2.0-sgen.lib
        ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysX_static_64.lib
        ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysXCharacterKinematic_static_64.lib
        ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysXCommon_static_64.lib
        ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysXCooking_static_64.lib
        ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysXExtensions_static_64.lib
        ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysXFoundation_static_64.lib
        ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysXPvdSDK_static_64.lib
    )
endif()

if(WIN32)
    list(APPEND LIB_LINKS d3d11 dxgi dxguid d3dCompiler)
endif()

# ── Target definition ─────────────────────────────────────────────────────────
add_library(Electro STATIC ${SOURCE_FILES})
target_include_directories(Electro PUBLIC ${INCLUDE_DIRS})
target_link_libraries(Electro PUBLIC ${LIB_LINKS})
target_precompile_headers(Electro PUBLIC "src/epch.hpp")

GroupSourcesByFolder(Electro)

set_property(TARGET Electro PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties(Electro PROPERTIES FOLDER Core)

# ── Compile definitions ───────────────────────────────────────────────────────
set(ELECTRO_COMPILE_DEFS
    "PX_PHYSX_STATIC_LIB"
    "GLM_FORCE_DEPTH_ZERO_TO_ONE"
    "PX_GENERATE_STATIC_LIBRARIES"
    $<$<CONFIG:Debug>:E_DEBUG>
    $<$<CONFIG:Release>:E_RELEASE>
)

if(ANDROID)
    list(APPEND ELECTRO_COMPILE_DEFS "ELECTRO_PLATFORM_ANDROID")
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    list(APPEND ELECTRO_COMPILE_DEFS "ELECTRO_PLATFORM_IOS")
else()
    list(APPEND ELECTRO_COMPILE_DEFS "ELECTRO_PLATFORM_WINDOWS" "ELECTRO_WINDOWS" "_CRT_SECURE_NO_WARNINGS")
endif()

target_compile_definitions(Electro PUBLIC ${ELECTRO_COMPILE_DEFS})

message(STATUS "Configuring Electro... done!")
