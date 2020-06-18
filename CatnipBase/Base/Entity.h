#pragma once
#include "Netvars.h"
#include "SDK/vector.h"

#define NV_RET(type, var) return *(type*)(this + var)

class CBaseEntity
{
public:
	inline Vector Origin() { NV_RET(Vector, Netvars::m_vecOrigin); }
	inline QAngle Rotation() { NV_RET(QAngle, Netvars::m_angRotation); }
	inline int Team() { NV_RET(int, Netvars::m_iTeamNum); }
};