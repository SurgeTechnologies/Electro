# Copyright 2021 - Electro Team - All Rights Reserved

# Example Project, this file will be included in the client
# Do not alter this file

cmake_minimum_required(VERSION 3.10)
file(GLOB_RECURSE ENGINE_SOURCE_FILES ../ElectroScript-Core/src/*.cs)
project("ElectroScript-Core" LANGUAGES CSharp CXX)
add_library("ElectroScript-Core" SHARED ${ENGINE_SOURCE_FILES})

set_target_properties("ElectroScript-Core"
     PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)

file(GLOB_RECURSE APP_SOURCE_FILES src/*.cs)
project("ExampleApp" LANGUAGES CSharp CXX)
add_library("ExampleApp" SHARED ${APP_SOURCE_FILES})
target_link_libraries("ExampleApp" ElectroScript-Core)

set_target_properties("ExampleApp"
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)