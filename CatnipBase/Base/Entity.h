#pragma once
#include "Netvars.h"
#include "SDK/vector.h"

#define NV_RET(type, var) return *(type*)((char*)this + var)

class CBaseEntity
{
public:
	inline Vector Origin() { NV_RET(Vector, Netvars::baseentity.m_vecOrigin); }
};