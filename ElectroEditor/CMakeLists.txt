# Copyright 2021 - Electro Team - All Rights Reserved
include(${CMAKE_SOURCE_DIR}/scripts/CMakeUtils.cmake)

set(INCLUDE_DIRS src Electro/vendor)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(ElectroEditor ${SOURCE_FILES})
target_link_libraries(ElectroEditor PRIVATE Electro)
target_include_directories(ElectroEditor PRIVATE ${INCLUDE_DIRS})

make_directory(${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)

#Copy the dlls to the binary directory
add_custom_command(
    TARGET ElectroEditor
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/Electro/vendor/assimp/lib/assimp-vc142-mt.dll
        ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>
    )
add_custom_command(
    TARGET ElectroEditor
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/Electro/vendor/ElectroMono/bin/Release/mono-2.0-sgen.dll
        ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>
    )

add_custom_command(
    TARGET ElectroEditor
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/Electro/vendor/glslc/glslc.exe
        ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>
    )

GroupSourcesByFolder(ElectroEditor)
set_property(TARGET ElectroEditor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties (ElectroEditor PROPERTIES FOLDER Tools)

message(STATUS "Configuring ElectroEditor... done!")