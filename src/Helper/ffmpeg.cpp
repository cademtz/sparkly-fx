#include "ffmpeg.h"
#include <Base/Base.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <array>
#include <optional>

namespace Helper::FFmpeg
{

namespace stdfs = std::filesystem;

static std::optional<stdfs::path> GetPathFromEnv(LPCWSTR var_name)
{
    DWORD length = MAX_PATH + 1;
    std::vector<WCHAR> buffer;
    
    while (true) // In the rare case that an environment variable keeps changing...
    {
        buffer.resize(length);
        length = GetEnvironmentVariableW(var_name, buffer.data(), buffer.size());
        if (length == 0)
            return std::nullopt;
        else if (length < buffer.size())
            return { buffer.data() };
    }
}

static std::optional<stdfs::path> GetPathFromModule(HMODULE module)
{
    DWORD length = MAX_PATH + 1;
    std::vector<WCHAR> buffer;
    
    while (true) // In the rare case that an environment variable keeps changing...
    {
        buffer.resize(length);
        length = GetModuleFileNameW(module, buffer.data(), buffer.size());
        if (length == 0)
            return std::nullopt;
        else if (length < buffer.size())
            return { buffer.data() };
        length = buffer.size() + 512;
    }
}

static void AppendIfExists(std::vector<stdfs::path>* vector, stdfs::path&& path)
{
    if (std::filesystem::exists(path))
        vector->emplace_back(std::move(path));
}

std::vector<std::filesystem::path> ScanForExecutables()
{
    std::vector<stdfs::path> results;
    std::optional<stdfs::path> programs_dir = GetPathFromEnv(L"ProgramW6432");
    std::optional<stdfs::path> programsx86_dir = GetPathFromEnv(L"ProgramFiles(x86)");
    
    // Check the current directory
    AppendIfExists(&results, "ffmpeg.exe");
    
    // Check our module's current directory
    AppendIfExists(&results, Base::GetModuleDir() / "ffmpeg.exe");

    // Check the HLAE module's current directory
    if (HMODULE hlae = GetModuleHandleW(L"AfxHookSource.dll"))
    {
        if (std::optional<stdfs::path> path = GetPathFromModule(hlae))
            AppendIfExists(&results, *path / "ffmpeg\\bin\\ffmpeg.exe");
    }

    // Check the install directories of common software.
    const WCHAR* const common_programs[] = {
        L"HLAE\\ffmpeg\\bin\\ffmpeg.exe",
        L"ShareX\\ffmpeg.exe"
    };

    for (const WCHAR* subpath : common_programs)
    {
        if (programs_dir)
            AppendIfExists(&results, *programs_dir / subpath);
        if (programsx86_dir)
            AppendIfExists(&results, *programsx86_dir / subpath);
    }

    // Finally, erase duplicates
    for (size_t i = 1; i < results.size(); ++i)
    {
        if (std::filesystem::equivalent(results[i - 1], results[i]))
        {
            results.erase(results.begin() + i);
            --i;
        }
    }

    return results;
}

}