#pragma once
#include "SDK/vector.h"

namespace Netvars
{
	void GetNetvars();

	inline struct {
		int m_vecOrigin;
		int m_angRotation;
		int m_iTeamNum;
	} baseentity;
}