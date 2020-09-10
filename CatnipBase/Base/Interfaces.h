#pragma once
#include "Wrappers/Wrappers.h"
#include "SDK/icliententitylist.h"
#include "SDK/IPanel.h"
#include "SDK/vgui_baseui_interface.h"

namespace Interfaces
{
	void CreateInterfaces();
	void DestroyInterfaces();

	inline IEngineClientWrapper* engine = nullptr;
	inline IClientDLLWrapper* hlclient = nullptr;
	inline IClientEntityListWrapper* entlist = nullptr;
	inline IClientModeWrapper* client = nullptr;
	inline vgui::IPanel* panels = nullptr;
	inline CEngineVGUIWrapper* vgui = nullptr;
}