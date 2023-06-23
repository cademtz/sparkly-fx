#pragma once
#include <Modules/BaseModule.h>
#include <array>

class WorldModule : public CModule
{
protected:
    void StartListening() override;
    int OnFrameStageNotify();
    int OnDrawPropArray();
    int OnDrawProp();
    int OnMenu();

public:
    bool m_draw_props = true;
    std::array<float, 3> m_colormod = { 1.f };
};