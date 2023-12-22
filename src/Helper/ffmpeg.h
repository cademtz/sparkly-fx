#pragma once
#include <vector>
#include <filesystem>
#include <string>
#include <string_view>
#include <memory>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Helper::FFmpeg
{

/**
 * @brief Scan the current drive/installation for FFmpeg executables.
 * 
 * The result is a list sorted by most relevance, where paths associated with the game and with Sparkly FX are most relevant.
 */
std::vector<std::filesystem::path> ScanForExecutables();

/**
 * @brief Get the default/preferred FFmpeg path. Thread-safe.
 * @return A blank path if no default is set.
 * @see SetDefaultPath
 */ 
std::filesystem::path GetDefaultPath();
/// @brief Set the default/preferred FFmpeg path. Thread-safe.
void SetDefaultPath(std::filesystem::path&& path);
/// @brief Set the default/preferred FFmpeg path. Thread-safe.
inline void SetDefaultPath(const std::filesystem::path& path) {
    return SetDefaultPath(std::filesystem::path(path));
}

/// @brief True when an AMD device with hardware encoding is present.
/// @details This will attempt to load a library. Do not call from DllMain.
bool amf_device_available();
/// @brief True when an NVIDIA device with hardware encoding is present
bool nvenc_device_available();

}