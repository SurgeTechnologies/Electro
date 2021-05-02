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
    vendor/yaml-cpp/include
    vendor/ElectroMono/include
    vendor/PhysX/include
    vendor/SPIR-V/include
)

set(LIB_LINKS
    imgui
    opengl32
    yaml-cpp

    ${CMAKE_SOURCE_DIR}/Electro/vendor/assimp/lib/assimp-vc142-mt.lib
    ${CMAKE_SOURCE_DIR}/Electro/vendor/ElectroMono/lib/mono-2.0-sgen.lib
    ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysX_static_64.lib
    ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysXCharacterKinematic_static_64.lib
    ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysXCommon_static_64.lib
    ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysXCooking_static_64.lib
    ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysXExtensions_static_64.lib
    ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysXFoundation_static_64.lib
    ${CMAKE_SOURCE_DIR}/Electro/vendor/PhysX/lib/$<CONFIG>/PhysXPvdSDK_static_64.lib

    ${CMAKE_SOURCE_DIR}/Electro/vendor/SPIR-V/lib/shaderc_shared.lib
    ${CMAKE_SOURCE_DIR}/Electro/vendor/SPIR-V/lib/spirv-cross-core.lib
    ${CMAKE_SOURCE_DIR}/Electro/vendor/SPIR-V/lib/spirv-cross-glsl.lib
    ${CMAKE_SOURCE_DIR}/Electro/vendor/SPIR-V/lib/spirv-cross-hlsl.lib
)

if(WIN32)
    set(DX11_LIBS d3d11 dxgi dxguid d3dCompiler)
else()
    set(DX11_LIBS)
endif()

file(GLOB_RECURSE SOURCE_FILES
    src/*.cpp
    src/*.hpp
    vendor/stb_image/stb_image.h
    vendor/stb_image/stb_image.cpp
)

add_library(Electro STATIC ${SOURCE_FILES})
target_include_directories(Electro PUBLIC ${INCLUDE_DIRS})
target_link_libraries(Electro PUBLIC ${LIB_LINKS} ${DX11_LIBS})
target_precompile_headers(Electro PUBLIC "src/epch.hpp")
set_property(TARGET Electro PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
GroupSourcesByFolder(Electro)

target_compile_definitions(
    Electro

    PRIVATE
    "_CRT_SECURE_NO_WARNINGS"
    "GLFW_INCLUDE_NONE"
    "PX_PHYSX_STATIC_LIB"

    PUBLIC
    "ELECTRO_WINDOWS"
    "PX_GENERATE_STATIC_LIBRARIES"
    $<$<CONFIG:Debug>:E_DEBUG>
    $<$<CONFIG:Release>:E_RELEASE>
)
