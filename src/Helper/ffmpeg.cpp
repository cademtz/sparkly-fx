#include "ffmpeg.h"
#include <Base/Base.h>
#include <array>
#include <optional>
#include <dxgi.h>
#include <mutex>

#pragma comment(lib, "windowscodecs.lib") // Link `IID_IDXGIFactory1`

namespace Helper::FFmpeg
{

namespace stdfs = std::filesystem;

static stdfs::path g_default_path;
static std::mutex g_default_path_mutex;

stdfs::path GetDefaultPath()
{
    const auto& list = GetDefaultPaths();
    if (list.empty())
        return {};
    return list.front();
}
void SetDefaultPath(stdfs::path&& path)
{
    std::scoped_lock lock(g_default_path_mutex);
    g_default_path = path;
}

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

const std::vector<stdfs::path>& GetDefaultPaths() {
    static std::vector<stdfs::path> paths = ScanForExecutables();
    return paths;
}

#if defined(_WIN32)
    #if defined(_M_AMD64)
        #define AMF_DLL_NAME    L"amfrt64.dll"
        #define AMF_DLL_NAMEA   "amfrt64.dll"
    #else
        #define AMF_DLL_NAME    L"amfrt32.dll"
        #define AMF_DLL_NAMEA   "amfrt32.dll"
    #endif
#endif

bool amf_device_available()
{
    static bool init = true;
    static bool available = false;

    if (init)
    {
        init = false;
        HMODULE handle = LoadLibraryW(AMF_DLL_NAME);
        available = handle != 0;
        if (handle)
            FreeLibrary(handle);
    }

    return available;
}

typedef HRESULT(WINAPI *create_dxgi_proc)(const IID *, IDXGIFactory1 **);

static const int blacklisted_adapters[] = {
    0x1298, // GK208M [GeForce GT 720M]
    0x1140, // GF117M [GeForce 610M/710M/810M/820M / GT 620M/625M/630M/720M]
    0x1293, // GK208M [GeForce GT 730M]
    0x1290, // GK208M [GeForce GT 730M]
    0x0fe1, // GK107M [GeForce GT 730M]
    0x0fdf, // GK107M [GeForce GT 740M]
    0x1294, // GK208M [GeForce GT 740M]
    0x1292, // GK208M [GeForce GT 740M]
    0x0fe2, // GK107M [GeForce GT 745M]
    0x0fe3, // GK107M [GeForce GT 745M]
    0x1140, // GF117M [GeForce 610M/710M/810M/820M / GT 620M/625M/630M/720M]
    0x0fed, // GK107M [GeForce 820M]
    0x1340, // GM108M [GeForce 830M]
    0x1393, // GM107M [GeForce 840M]
    0x1341, // GM108M [GeForce 840M]
    0x1398, // GM107M [GeForce 845M]
    0x1390, // GM107M [GeForce 845M]
    0x1344, // GM108M [GeForce 845M]
    0x1299, // GK208BM [GeForce 920M]
    0x134f, // GM108M [GeForce 920MX]
    0x134e, // GM108M [GeForce 930MX]
    0x1349, // GM108M [GeForce 930M]
    0x1346, // GM108M [GeForce 930M]
    0x179c, // GM107 [GeForce 940MX]
    0x139c, // GM107M [GeForce 940M]
    0x1347, // GM108M [GeForce 940M]
    0x134d, // GM108M [GeForce 940MX]
    0x134b, // GM108M [GeForce 940MX]
    0x1399, // GM107M [GeForce 945M]
    0x1348, // GM108M [GeForce 945M / 945A]
    0x1d01, // GP108 [GeForce GT 1030]
    0x0fc5, // GK107 [GeForce GT 1030]
    0x174e, // GM108M [GeForce MX110]
    0x174d, // GM108M [GeForce MX130]
    0x1d10, // GP108M [GeForce MX150]
    0x1d12, // GP108M [GeForce MX150]
    0x1d11, // GP108M [GeForce MX230]
    0x1d13, // GP108M [GeForce MX250]
    0x1d52, // GP108BM [GeForce MX250]
    0x1c94, // GP107 [GeForce MX350]
    0x1c96, // GP107 [GeForce MX350]
    0x1f97, // TU117 [GeForce MX450]
    0x1f98, // TU117 [GeForce MX450]
    0x137b, // GM108GLM [Quadro M520 Mobile]
    0x1d33, // GP108GLM [Quadro P500 Mobile]
    0x137a, // GM108GLM [Quadro K620M / Quadro M500M]
};


static bool is_blacklisted(const int device_id)
{
    for (int blacklisted_adapter : blacklisted_adapters)
    {
        if (device_id == blacklisted_adapter)
            return true;
    }

    return false;
}

bool nvenc_device_available()
{
    static HMODULE dxgi = LoadLibraryW(L"dxgi");
    static create_dxgi_proc create = NULL;
    IDXGIFactory1 *factory;
    IDXGIAdapter1 *adapter;
    bool available = false;
    HRESULT hr;
    UINT i = 0;

    if (!dxgi)
        return true;

    if (!create)
    {
        create = (create_dxgi_proc)GetProcAddress(dxgi, "CreateDXGIFactory1");
        if (!create)
        {
            return true;
        }
    }

    hr = create(&IID_IDXGIFactory1, &factory);
    if (FAILED(hr))
    {
        return true;
    }

    while (factory->EnumAdapters1(i++, &adapter) == S_OK)
    {
        DXGI_ADAPTER_DESC desc;

        hr = adapter->GetDesc(&desc);
        adapter->Release();

        if (FAILED(hr))
            continue;
        
        // 0x10de = NVIDIA Corporation
        if (desc.VendorId == 0x10de && !is_blacklisted(desc.DeviceId))
        {
            available = true;
            break;
        }
    }

    factory->Release();
    return available;
}

}