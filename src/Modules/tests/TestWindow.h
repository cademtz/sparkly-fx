#pragma once
#include <Modules/BaseModule.h>

class TestWindow : public CModule {
public:
    void StartListening() override;

    /// @brief Runs when the test window is being drawn
    static inline EventSource<void()> OnWindow;

private:
    int OnImGui();
};