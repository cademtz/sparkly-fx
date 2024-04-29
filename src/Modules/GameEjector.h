#pragma once
#include "BaseModule.h"
#include <SDK/cdll_int.h> // ClientFrameStage_t

/**
 * @brief Ejects this DLL from the game.
 * Useful for quickly recompiling and testing code.
 */
class GameEjector : public CModule
{
public:
    void StartListening() override;

    /// @brief Runs on the game thread before ejecting
    static inline EventSource<void()> OnEject;
    
    /// Eject from the game
    static void Eject();

private:
    int OnFrameStageNotify(ClientFrameStage_t stage);
};