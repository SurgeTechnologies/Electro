# Copyright(c) 2021 - Electro Team - All rights reserved
# Android NDK CMakeLists.txt – builds ElectroRuntime as a shared library
# loaded by NativeActivity.
#
# Usage (invoked automatically by Gradle, or manually):
#   cmake -DANDROID=1 -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26 \
#         -DANDROID_STL=c++_shared                                           \
#         -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
#         ../../../..   # repository root

cmake_minimum_required(VERSION 3.22)
project(ElectroRuntime)

set(CMAKE_CXX_STANDARD 17)

# Repository root is four levels above this file
set(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../..")

add_subdirectory(${REPO_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/electro_build)

# ── ElectroRuntime shared library ────────────────────────────────────────────
# The actual game / runtime entry-point is supplied by the ElectroRuntime
# CMakeLists in the repo root.  Here we just set the output type so Android
# can dlopen it from NativeActivity.
set_target_properties(ElectroRuntime PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
)
