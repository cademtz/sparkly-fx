#pragma once

#include <Modules/Menu.h>
#include <imgui.h>

class TestCrash : public CModule
{
public:
	TestCrash() { }
	void StartListening() override {
        Listen(EVENT_MENU, [this] { return OnMenu(); });
    }

private:
	int OnMenu()
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