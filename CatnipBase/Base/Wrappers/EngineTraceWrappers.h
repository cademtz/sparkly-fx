#pragma once
#include "Wrappers.h"

class IEngineTrace;
class IEngineTrace004;

class IEngineTrace003Wrapper : public IEngineTraceWrapper
{
	IEngineTrace* m_int;

public:
	IEngineTrace003Wrapper(void* EngineTrace) : m_int((IEngineTrace*)EngineTrace) { }
	virtual void* Inst() { return m_int; }

	virtual int		GetPointContents(const Vector& vecAbsPosition, IHandleEntity** ppEntity = NULL);
	virtual void	TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace);
};

class IEngineTrace004Wrapper : public IEngineTraceWrapper
{
	IEngineTrace004* m_int;

public:
	IEngineTrace004Wrapper(void* EngineTrace) : m_int((IEngineTrace004*)EngineTrace) { }
	virtual void* Inst() { return m_int; }

	virtual int		GetPointContents(const Vector& vecAbsPosition, IHandleEntity** ppEntity = NULL);
	virtual void	TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace);
};
