# Copyright 2021 - Electro Team - All Rights Reserved
include(${CMAKE_SOURCE_DIR}/scripts/CMakeUtils.cmake)

set(INCLUDE_DIRS Source Electro/vendor)
file(GLOB_RECURSE SOURCE_FILES Source/*.cpp Source/*.hpp)

add_executable(ElectroRuntime ${SOURCE_FILES})
target_link_libraries(ElectroRuntime PRIVATE Electro)
target_include_directories(ElectroRuntime PRIVATE ${INCLUDE_DIRS})

make_directory(${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)

#Copy the dlls to the binary directory
add_custom_command(
    TARGET ElectroRuntime
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/Electro/vendor/assimp/lib/assimp-vc142-mt.dll
        ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>
    )
add_custom_command(
    TARGET ElectroRuntime
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/Electro/vendor/ElectroMono/bin/Release/mono-2.0-sgen.dll
        ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>
    )

make_directory(${CMAKE_CURRENT_SOURCE_DIR}/Bin)
add_custom_command(
    TARGET ElectroRuntime
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/ElectroRuntime.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/Bin
    )
add_custom_command(
    TARGET ElectroRuntime
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/Electro/vendor/assimp/lib/assimp-vc142-mt.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/Bin
    )

add_custom_command(
    TARGET ElectroRuntime
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/Electro/vendor/ElectroMono/bin/Release/mono-2.0-sgen.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/Bin
    )

GroupSourcesByFolder(ElectroRuntime)
set_property(TARGET ElectroRuntime PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties (ElectroRuntime PROPERTIES FOLDER Core)

message(STATUS "Configuring ElectroRuntime... done!")