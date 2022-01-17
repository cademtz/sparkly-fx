#pragma once
#include <SDK/IEngineTrace.h>

class IEngineTrace004
{
public:
	// Returns the contents mask + entity at a particular world-space position
	virtual int		GetPointContents(const Vector& vecAbsPosition, IHandleEntity** ppEntity = NULL) = 0;
	virtual void _unk1() = 0;
	virtual void _unk2() = 0;
	virtual void _unk3() = 0;
	virtual void _unk4() = 0;
	// A version that simply accepts a ray (can work as a traceline or tracehull)
	virtual void	TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) = 0;
};
