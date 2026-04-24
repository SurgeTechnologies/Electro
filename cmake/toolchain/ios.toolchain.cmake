# Copyright(c) 2021 - Electro Team - All rights reserved
# ios.toolchain.cmake – minimal iOS cross-compilation toolchain for Electro.
#
# This file configures CMake to cross-compile for iOS (arm64 device builds).
# For a production-grade toolchain with simulator support, fat-binary (lipo)
# creation, and code-signing integration, consider using the community
# toolchain from https://github.com/leetal/ios-cmake and passing:
#
#   -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/ios.toolchain.cmake
#   -DPLATFORM=OS64         # arm64 device
#
# ─────────────────────────────────────────────────────────────────────────────

cmake_minimum_required(VERSION 3.16)

# Prevent CMake from trying to test the compiler with a full executable link
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ── System identification ─────────────────────────────────────────────────────
set(CMAKE_SYSTEM_NAME      iOS)
set(CMAKE_SYSTEM_PROCESSOR arm64)

# ── Xcode / Apple-clang paths ────────────────────────────────────────────────
# Find Xcode's iOS SDK automatically via xcrun.
execute_process(
    COMMAND xcrun --sdk iphoneos --show-sdk-path
    OUTPUT_VARIABLE IOS_SDK_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT IOS_SDK_PATH)
    message(FATAL_ERROR "Could not locate the iOS SDK via xcrun. Make sure Xcode is installed and the iOS SDK is available.")
endif()

set(CMAKE_OSX_SYSROOT     "${IOS_SDK_PATH}" CACHE STRING "iOS SDK sysroot")
set(CMAKE_OSX_ARCHITECTURES "arm64"         CACHE STRING "Build for arm64 only")

# Minimum deployment target
if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum iOS version")
endif()

# ── Compiler selection ────────────────────────────────────────────────────────
find_program(CMAKE_C_COMPILER   clang REQUIRED)
find_program(CMAKE_CXX_COMPILER clang++ REQUIRED)

set(CMAKE_C_COMPILER_TARGET   "arm64-apple-ios${CMAKE_OSX_DEPLOYMENT_TARGET}")
set(CMAKE_CXX_COMPILER_TARGET "arm64-apple-ios${CMAKE_OSX_DEPLOYMENT_TARGET}")

# ── Skip shared-library support (iOS only supports static libs + frameworks) ─
set(BUILD_SHARED_LIBS OFF CACHE BOOL "iOS only supports static libs")

# ── Bitcode (optional; required by older App Store rules) ────────────────────
# Enable with: -DENABLE_BITCODE=ON
option(ENABLE_BITCODE "Embed bitcode in compiled objects" OFF)
if(ENABLE_BITCODE)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fembed-bitcode")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fembed-bitcode")
endif()

# ── Prevent macOS-only paths polluting the search ────────────────────────────
set(CMAKE_FIND_ROOT_PATH        "${IOS_SDK_PATH}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
