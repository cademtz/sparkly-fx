#pragma once

#include "TestWindow.h"
#include <Modules/GameEjector.h>
#include <imgui.h>

class TestEject : public CModule
{
public:
    void StartListening() override {
        TestWindow::OnWindow.Listen(&TestEject::OnWindow, this);
    }

private:
    int OnWindow()
    {
        if (ImGui::Button("Eject"))
            GameEjector::Eject();
        return 0;
    }
};

static TestEject g_test_ejector;