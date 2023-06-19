#include "MaterialSystemWrappers.h"
#include <SDK/imaterialsystem.h>
#include <SDK/InterfaceVersions/MaterialSystem081.h>

IMatRenderContext* IMaterialSystemWrapperSDK::GetRenderContext() {
    return m_int->GetRenderContext();
}

IMatRenderContext* IMaterialSystemWrapper081::GetRenderContext() {
    return m_int->GetRenderContext();
}
