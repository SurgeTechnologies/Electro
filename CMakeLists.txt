# Copyright(c) 2021 - Electro Team - All rights reserved
cmake_minimum_required(VERSION 3.10)

project(Electro)
set(CMAKE_CXX_STANDARD 17)

#Prevents in source build
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -g3 -O0 -Wall -pedantic -Wextra -m64 -mavx2 -mfma -ffast-math") #Using Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -Wall -pedantic -Wextra -m64 -mavx2 -mfma -ffast-math") #Using GCC
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /std:c++17 /W4 /arch:AVX2 /fp:fast /MP /DNOMINMAX") #Using Visual Studio C++
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

add_subdirectory(Electro/vendor/imgui)
add_subdirectory(Electro/vendor/Yaml)
add_subdirectory(Electro/vendor/SPIRV-Cross)

add_subdirectory(Electro)
add_subdirectory(ElectroEditor)
add_subdirectory(ElectroScript-Core)
