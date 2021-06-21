# Copyright(c) 2021 - Electro Team - All rights reserved
include(${CMAKE_SOURCE_DIR}/scripts/CMakeUtils.cmake)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "include/*.h")
set(INCLUDE_DIRS include)

add_library(Meshoptimizer ${SOURCE_FILES})
include_directories(${INCLUDE_DIRS})

GroupSourcesByFolder(Meshoptimizer)
set_target_properties (Meshoptimizer PROPERTIES FOLDER Dependencies)

message(STATUS "Configuring Meshoptimizer... done!")