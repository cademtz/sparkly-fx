#include "TestWindow.h"
#include <Modules/Draw.h>
#include <Modules/InputModule.h>

static TestWindow g_test_window;

void TestWindow::StartListening() {
    CDraw::OnImGui.Listen(&TestWindow::OnImGui, this);
}

int TestWindow::OnImGui() {
    if (!g_input.IsOverlayOpen())
        return 0;
    
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Once);
    if (ImGui::Begin("Test window"))
        OnWindow.DispatchEvent();
    ImGui::End();
    return 0;
}