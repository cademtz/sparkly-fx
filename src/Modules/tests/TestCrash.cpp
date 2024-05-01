#pragma once

#include "TestWindow.h"
#include <imgui.h>

class TestCrash : public CModule
{
public:
    void StartListening() override {
        TestWindow::OnWindow.Listen(&TestCrash::OnWindow, this);
    }

private:
    int OnWindow()
    {
        if (ImGui::CollapsingHeader("Crash test"))
        {
            if (ImGui::Button("Crash"))
            {
                volatile int* evil = 0;
                *evil = 99;
            }
        }
        return 0;
    }
};

static TestCrash g_test_crash;