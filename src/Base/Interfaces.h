#pragma once
#include "Wrappers/Wrappers.h"

namespace vgui { class IPanel; }
class IEngineTool;
class IStudioRender;

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
	inline IEngineTraceWrapper* trace = nullptr;
	inline IEngineTool* engine_tool = nullptr;
	inline IMaterialSystemWrapper* mat_system;
	inline IStudioRender* studio_render;
	inline IVModelRenderWrapper* model_render = nullptr;
}