# Copyright(c) 2021 - Electro Team - All rights reserved

set(INCLUDE_DIRS
    src
)

set(LIB_LINKS
    opengl32
)

if(WIN32)
    set(DX11_LIBS d3d11 dxgi dxguid d3dCompiler)
else()
    set(DX11_LIBS)
endif()

file(GLOB_RECURSE SOURCE_FILES
    src/*.cpp
    src/*.hpp
)

add_library(Electro SHARED ${SOURCE_FILES})
target_include_directories(Electro PUBLIC ${INCLUDE_DIRS})
target_link_libraries(Electro PUBLIC ${LIB_LINKS} ${DX11_LIBS})
target_precompile_headers(Electro PUBLIC "src/epch.hpp")
set_property(TARGET Electro PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

target_compile_definitions(
    Electro

    PRIVATE
    "E_BUILD_DLL"
    "_CRT_SECURE_NO_WARNINGS"
    "GLFW_INCLUDE_NONE"

    PUBLIC
    $<$<CONFIG:Debug>:E_DEBUG>
    $<$<CONFIG:Release>:E_RELEASE>

    $<$<CONFIG:RelWithDebInfo>:E_DEBUG>
    $<$<CONFIG:MinSizeRel>:E_RELEASE>
)
