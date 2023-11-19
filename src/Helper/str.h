#pragma once
#include <cstring>

namespace Helper
{
    /// @brief Case-insensitive search for `substr` in `str`
    /// @return The first occurence of `substr` in `str`, or `nullptr`
    template <class T = char>
    const T* strcasestr(const T* str, const T* substr)
    {
        while (*str)
        { 
            for (size_t i = 0; ; ++i)
            {
                if (!substr[i])
                    return str;
                if (std::tolower(str[i]) != std::tolower(substr[i]))
                    break;
            }
            ++str;
        }
        return nullptr;
    }
}