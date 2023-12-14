#include "str.h"
#include <cctype>
#include <cstdarg>
#include <vector>

namespace Helper
{

std::string sprintf(const char* fmt, ...)
{
    std::string buffer(256, '\0');

    bool is_incomplete = true;
    do
    {
        va_list va;
        va_start(va, fmt);
        int len = std::vsnprintf(buffer.data(), buffer.size() + 1 /* Includes null-terminator */, fmt, va);
        va_end(va);

        is_incomplete = len > buffer.size();
        buffer.resize(len);
    } while (is_incomplete);
    return buffer;
}

size_t ParseCommandName(const char* str, std::string_view* output)
{
    if (!str[0])
        return 0;
    const char* end = str;
    while (std::isalpha(*end) || std::isdigit(*end) || *end == '_')
        ++end;
    *output = {str, (size_t)(end - str)};
    return output->length();
}

size_t ParseCommandArgs(const char* str, std::string_view* output)
{
    if (!str[0])
        return 0;
    size_t len = 0;
    for (size_t i = 0; ; ++i)
    {
        if (str[i] == '"') // Skip to end quote
        {
            do
            {
                if (str[i] != ' ' && str[i] != ';')
                    len = i + 1; // Update length now in case end quote isn't found later...
                if (str[i] == '\\' && (str[i] == '"' || str[i] == '\\'))
                    ++i; // Skip the escaped character
                ++i;
            } while (str[i] && str[i] != '"' && str[i] != '\n');
        }
        if (!str[i] || str[i] == ';' || str[i] == '\n')
            break;
        else if (str[i] != ' ')
            len = i + 1; // Update length only for non-whitespace characters
    }
    *output = {str, len};
    return len;
}

size_t ParseNextCommand(const char* str, ParsedCommand* output)
{
    const char* begin = str;
    while (*str == ' ' || *str == '\n' || *str == ';') // Skip junk chars
        ++str;
    
    size_t read = ParseCommandName(str, &output->name);
    if (!read)
        return 0;

    str += read;
    while (*str == ' ')
        ++str;
    
    output->args = {};
    read = ParseCommandArgs(str, &output->args);
    str += read;
    
    return str - begin;
}

}