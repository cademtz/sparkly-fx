#include "mainwindow.h"
#include <Modules/Draw.h>
#include <Modules/InputModule.h>

static MainWindow g_main_window;

void MainWindow::StartListening() {
    CDraw::OnImGui.Listen(&MainWindow::OnImGui, this);
}

int MainWindow::OnImGui() {
    if (!g_input.IsOverlayOpen())
        return 0;
    
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Once);
    if (ImGui::Begin("Sparkly FX"))
        OnWindow.DispatchEvent();
    ImGui::End();
    return 0;
}