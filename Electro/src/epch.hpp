//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <filesystem>
#include "Core/Base.hpp"
#include "Core/Log.hpp"

#ifdef ELECTRO_PLATFORM_WINDOWS
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif
