# Copyright(c) 2021 - Electro Team - All rights reserved

set(INCLUDE_DIRS src)
set(LIB_LINKS Electro)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(ExampleApp ${SOURCE_FILES})
target_include_directories(ExampleApp PRIVATE ${INCLUDE_DIRS})
target_link_libraries(ExampleApp PRIVATE ${LIB_LINKS})
set_property(TARGET ExampleApp PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_command(TARGET ExampleApp PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/Electro/Debug/Electro.dll ${CMAKE_BINARY_DIR}/ExampleApp/Debug)
add_custom_command(TARGET ExampleApp PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/Electro/Release/Electro.dll ${CMAKE_BINARY_DIR}/ExampleApp/Release)