#pragma once
#include <Modules/BaseModule.h>

class WorldModule : public CModule
{
protected:
    void StartListening() override;
    int OnDrawPropArray();
    int OnDrawProp();
    int OnMenu();

public:
    bool m_draw_props = true;
};