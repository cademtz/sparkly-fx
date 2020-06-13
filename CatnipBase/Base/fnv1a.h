#pragma once
#include <cstdint>

namespace FNV
{
    namespace internal
    {
        constexpr static uint32_t default_offset_basis = 0x811C9DC5;
        constexpr static uint32_t prime = 0x01000193;
    }

    constexpr static inline uint32_t calculate(char const* const pszString, const uint32_t val = internal::default_offset_basis)
    {
        return (pszString[0] == '\0') ? val : calculate(&pszString[1], (val * internal::prime) ^ uint32_t(pszString[0]));
    }
};

constexpr static inline uint32_t operator ""_hash(const char* pszString, size_t)
{
    return FNV::calculate(pszString);
}