# Copyright(c) 2021 - Electro Team - All rights reserved

set(INCLUDE_DIRS
    src
    vendor/imgui
    vendor/stb_image
    vendor/assimp/include
    vendor/glm
)

set(LIB_LINKS
    imgui
    opengl32
    ${CMAKE_SOURCE_DIR}/Electro/vendor/assimp/lib/assimp-vc142-mt.lib
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

target_compile_definitions(
    Electro

    PRIVATE
    "_CRT_SECURE_NO_WARNINGS"
    "GLFW_INCLUDE_NONE"

    PUBLIC
    "ELECTRO_WINDOWS"

    $<$<CONFIG:Debug>:E_DEBUG>
    $<$<CONFIG:Release>:E_RELEASE>

    $<$<CONFIG:RelWithDebInfo>:E_DEBUG>
    $<$<CONFIG:MinSizeRel>:E_RELEASE>
)