#pragma once
#include <Base/Base.h>
#include <Modules/Draw.h>
#include <Hooks/WindowHook.h>

#include "BaseModule.h"

class CMenu : public CModule
{
public:
	CMenu();
	void StartListening() override;
	void SetOpen(bool Val);
	bool IsOpen() const;

	using MenuEvent = EventSource<void()>;
	using PostImguiInputEvent = EventSource<bool()>;

	static inline MenuEvent OnMenu;
	static inline PostImguiInputEvent OnPostImguiInput;
private:
	bool AcceptMsg(HWND hWnd, UINT uMsg, LPARAM lParam, WPARAM wParam);

	int OnImGui() const;
	int OnWindowProc(LRESULT& result, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool m_open = false;
};

inline CMenu g_menu;