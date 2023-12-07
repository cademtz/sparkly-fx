#pragma once
#include <vector>
#include <filesystem>

namespace Helper::FFmpeg
{

/**
 * @brief Scan the current drive/installation for FFmpeg executables.
 * 
 * The result is a list sorted by most relevance, where paths associated with the game and with Sparkly FX are most relevant.
 */
std::vector<std::filesystem::path> ScanForExecutables();

}