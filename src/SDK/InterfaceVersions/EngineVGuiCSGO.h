#pragma once
#include "SDK/ienginevgui.h"

struct InputEvent_t;

class CEngineVGuiCSGO
{
public:
	virtual ~CEngineVGuiCSGO(void) { }
	virtual vgui::VPANEL GetPanel(VGuiPanel_t type) = 0;
	virtual bool IsGameUIVisible() = 0;
	virtual void _unk3() = 0;
	virtual void Init() = 0;
	virtual void Connect() = 0;
	virtual void Shutdown() = 0;
	virtual bool SetVGUIDirectories() = 0;
	virtual bool IsInitialized() const = 0;
	virtual CreateInterfaceFn GetGameUIFactory() = 0;
	virtual bool Key_Event(const InputEvent_t& event) = 0;
	virtual void BackwardCompatibility_Paint() = 0;
	virtual void UpdateButtonState(const InputEvent_t& event) = 0;
	virtual void PostInit() = 0;
	virtual void Paint(PaintMode_t mode) = 0;
	virtual bool HideGameUI() = 0;
	virtual void ShowConsole() = 0;
	virtual void HideConsole() = 0;
	virtual void IsConsoleVisible() = 0;
	virtual void ClearConsole() = 0;
	// I messed up smthn horribly while reversing the rest. Unnecessary functions, anyways
};