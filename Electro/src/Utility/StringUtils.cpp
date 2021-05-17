//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"

namespace Electro::Utils
{
    Vector<String> SplitString(const String& string, const String& delimiters)
    {
        size_t start = 0;
        size_t end = string.find_first_of(delimiters);

        Vector<String> result;

        while (end <= String::npos)
        {
            String token = string.substr(start, end - start);
            if (!token.empty())
                result.push_back(token);

            if (end == String::npos)
                break;

            start = end + 1;
            end = string.find_first_of(delimiters, start);
        }

        return result;
    }

    bool StringEquals(const String& a, const String& b)
    {
        return std::equal(a.begin(), a.end(), b.begin(), b.end(),
            [](char a, char b)
            {
                return std::tolower(a) == std::tolower(b);
            });
    }

}
