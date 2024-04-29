#include "BaseModule.h"

std::list<CModule*>& CModule::GetModuleList() {
    static std::list<CModule*> list;
    return list;
}