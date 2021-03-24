# Copyright 2021 - Electro Team - All Rights Reserved
include(${CMAKE_SOURCE_DIR}/scripts/CMakeUtils.cmake)

set(INCLUDE_DIRS src Electro/vendor)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(ElectroEditor ${SOURCE_FILES})
target_link_libraries(ElectroEditor PRIVATE Electro)
target_include_directories(ElectroEditor PRIVATE ${INCLUDE_DIRS})
GroupSourcesByFolder(ElectroEditor)

file(COPY ${CMAKE_SOURCE_DIR}/Electro/vendor/assimp/lib/assimp-vc142-mt.dll DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")
file(COPY ${CMAKE_SOURCE_DIR}/Electro/vendor/assimp/lib/assimp-vc142-mt.dll DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Release")

set_property(TARGET ElectroEditor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ElectroEditor)
