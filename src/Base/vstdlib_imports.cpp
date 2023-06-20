/*
 * Purpose: Fill-in the functions and variables that are typically imported from vstdlib.dll
 */
#include "Base.h"
class IKeyValuesSystem;

IKeyValuesSystem *KeyValuesSystem()
{
    using fn_type = IKeyValuesSystem*();
    static void* fn = Base::GetProc(Base::GetModule("vstdlib.dll"), "KeyValuesSystem");
    return ((fn_type*)fn)();
}