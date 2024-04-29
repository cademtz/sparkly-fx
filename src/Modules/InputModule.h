#pragma once
#include <Base/Base.h>
#include <Modules/Draw.h>
#include <Hooks/WindowHook.h>

#include "BaseModule.h"

/**
 * @brief Handles overlay state and input for ImGui.
 * 
 * Use @ref IsOverlayOpen
 */
class InputModule : public CModule
{
public:
	InputModule();
	void StartListening() override;
	void SetOverlayOpen(bool Val);
	bool IsOverlayOpen() const;

	/// @brief Runs after ImGui has updated the input state, but before rendering
	static inline EventSource<bool()> OnPostImguiInput;

private:
	/**
	 * Handle input and decide whether to pass or block it from the game.
	 * @return `true` if the input is blocked and used for the overlay.
	 */
	bool AcceptMsg(HWND hWnd, UINT uMsg, LPARAM lParam, WPARAM wParam);
	int OnWindowProc(LRESULT& result, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool m_open = false;
};

inline InputModule g_input;