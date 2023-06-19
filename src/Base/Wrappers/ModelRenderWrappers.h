#pragma once
#include "Wrappers.h"

class IVModelRender;

class IVModelRenderWrapperSDK : public IVModelRenderWrapper
{
    IVModelRender* m_int;

public:
    IVModelRenderWrapperSDK(void* instance) : m_int((IVModelRender*)instance) {}

    void* Inst() override { return m_int; }
    int GetOffset(EOffsets Offset) override;
};
