# Copyright 2021 - Electro Team - All Rights Reserved
file(GLOB_RECURSE SOURCE_FILES src/*.cs)
project("ElectroScript-Core" LANGUAGES CSharp)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
message(STATUS "Configuring ElectroScript-Core... done!")