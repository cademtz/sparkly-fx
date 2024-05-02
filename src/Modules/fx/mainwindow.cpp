#include "mainwindow.h"
#include <Modules/Draw.h>
#include <Modules/InputModule.h>

static MainWindow g_main_window;

void MainWindow::StartListening() {
    CDraw::OnImGui.Listen(&MainWindow::OnImGui, this);
}

int MainWindow::OnImGui()
{
    if (!g_input.IsOverlayOpen())
        return 0;
    
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Once);
    if (ImGui::Begin("Sparkly FX", nullptr, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            OnMenuBar.DispatchEvent();
            ImGui::EndMenuBar();
        }
        if (ImGui::BeginTabBar("MainWindowTabs"))
        {
            OnTabBar.DispatchEvent();
            ImGui::EndTabBar();
        }
        OnWindow.DispatchEvent();
    }
    ImGui::End();
    return 0;
}