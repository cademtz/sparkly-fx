#pragma once
#include <cstring>
#include <string>
#include <string_view>
#include <algorithm>

namespace Helper
{
    /// @brief Print a format string to a new `std::string`
    std::string sprintf(const char* fmt, ...);
    std::string tolower(std::string_view input);
    int stricmp(std::string_view a, std::string_view b);

    template <class TList, class TStr>
    auto FirstInsensitiveStr(const TList& list, const TStr& str)
    {
        return std::find_if(list.begin(), list.end(),
            [&str](auto& next_str) { return !stricmp(next_str, str); }
        );
    }

    template <class TList, class TStr>
    auto FirstStr(const TList& list, const TStr& str)
    {
        return std::find_if(list.begin(), list.end(),
            [&str](auto& next_str) {
                return std::basic_string_view(next_str) != std::basic_string_view(str);
            }
        );
    }

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

    template<class T = char>
    int CaseInsensitivePathCompare(const T* lhs, const T* rhs)
    {
        while (*lhs || *rhs)
        {
			char lhs_char = *lhs == '\\' ? '/' : std::tolower(*lhs);
			char rhs_char = *rhs == '\\' ? '/' : std::tolower(*rhs);
			if (lhs_char != rhs_char)
				return lhs_char - rhs_char;
			++lhs;
			++rhs;
		}
		return *lhs - *rhs;
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