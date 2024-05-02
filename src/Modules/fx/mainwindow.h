#pragma once
#include <Modules/BaseModule.h>

class MainWindow : public CModule
{
public:
    void StartListening() override;

    /// @brief Runs when the test window is being drawn
    static inline EventSource<void()> OnWindow;
    /// @brief Add your tab item during this event
    static inline EventSource<void()> OnTabBar;
    /// @brief Add your menu bar during this event
    static inline EventSource<void()> OnMenuBar;

private:
    int OnImGui();
};