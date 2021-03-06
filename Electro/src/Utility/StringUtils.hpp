//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"

namespace Electro::Utils
{
    Vector<String> SplitString(const String& string, const String& delimiters);
    bool StringEquals(const String& a, const String& b);
    String ToLower(String& str);
    bool IsNumber(const String& str);
}
