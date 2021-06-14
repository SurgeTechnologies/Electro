# Copyright(c) 2021 - Electro Team - All rights reserved

include(${CMAKE_SOURCE_DIR}/scripts/CMakeUtils.cmake)

file(GLOB_RECURSE SOURCE_FILES "src/*.hpp" "src/*.cpp" "include/*.hpp")
set(INCLUDE_DIRS "include/SPIRV-Cross")

add_library(SPIRV-Cross ${SOURCE_FILES})
include_directories(${INCLUDE_DIRS})
GroupSourcesByFolder(SPIRV-Cross)

set_target_properties (SPIRV-Cross PROPERTIES FOLDER Dependencies)
message(STATUS "Configuring SPIRV-Cross... done!")