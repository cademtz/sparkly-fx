#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Base/Base.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
  switch (dwReasonForCall)
  {
  case DLL_PROCESS_ATTACH:
    Base::OnAttach(hModule);
    break;
  case DLL_PROCESS_DETACH:
    break;
  }

  return TRUE;
}
