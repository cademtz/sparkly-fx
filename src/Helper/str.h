#pragma once
#include <cstring>
#include <string_view>

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

    /// @brief Case-and-slash-insensitive search for `substr` in `str`
    /// @return The first occurence of `substr` in `str`, or `nullptr`
    template <class T = char>
    const T* CaseInsensitivePathSubstr(const T* str, const T* substr)
    {
        while (*str)
        { 
            for (size_t i = 0; ; ++i)
            {
                if (!substr[i])
                    return str;

                char lhs = str[i] == '\\' ? '/' : std::tolower(str[i]);
                char rhs = substr[i] == '\\' ? '/' : std::tolower(substr[i]);
                
                if (lhs != rhs)
                    break;
            }
            ++str;
        }
        return nullptr;
    }

    struct ParsedCommand
    {
        std::string_view name;
        /// @brief Command arguments. May be empty.
        std::string_view args;
    };

    /// @brief Find and parse the command
    /// @return The number of chars read, or 0 if there is no command.
    size_t ParseNextCommand(const char* str, ParsedCommand* output);
}