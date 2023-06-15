#pragma once
#include <cstdint>

namespace fnv
{
    namespace internal
    {
        constexpr static uint32_t default_offset_basis_32 = 0x811C9DC5;
        constexpr static uint64_t default_offset_basis_64 = 0xCBF29CE484222325;
        constexpr static uint32_t prime_32 = 0x01000193;
        constexpr static uint64_t prime_64 = 0x100000001B3;

		constexpr size_t default_offset_basis_z() {
			if constexpr (sizeof(size_t) <= sizeof(uint32_t))
				return default_offset_basis_32;
			else
				return default_offset_basis_64;
		}

		constexpr size_t prime_z() {
			if constexpr (sizeof(size_t) <= sizeof(uint32_t))
				return prime_32;
			else
				return prime_64;
		}
    }

    constexpr static inline uint32_t calculate_32(char const* const pszString, const uint32_t val = internal::default_offset_basis_32) {
        return (pszString[0] == '\0') ? val : calculate_32(&pszString[1], (val * internal::prime_32) ^ uint32_t(pszString[0]));
    }

	inline size_t calculate(size_t lastHash, size_t val) {
		return (lastHash ^ val) * internal::prime_z();
	}
};

constexpr static inline uint32_t operator ""_hash(const char* pszString, size_t)
{
    return fnv::calculate_32(pszString);
}