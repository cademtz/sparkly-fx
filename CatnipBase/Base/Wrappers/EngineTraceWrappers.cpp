#include "EngineTraceWrappers.h"
#include <SDK/InterfaceVersions/EngineTrace003.h>
#include <SDK/InterfaceVersions/EngineTrace004.h>

int IEngineTrace003Wrapper::GetPointContents(const Vector& vecAbsPosition, IHandleEntity** ppEntity) {
    return m_int->GetPointContents(vecAbsPosition, ppEntity);
}
void IEngineTrace003Wrapper::TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) {
	m_int->TraceRay(ray, fMask, pTraceFilter, pTrace);
}

int IEngineTrace004Wrapper::GetPointContents(const Vector& vecAbsPosition, IHandleEntity** ppEntity) {
    return m_int->GetPointContents(vecAbsPosition, ppEntity);
}
void IEngineTrace004Wrapper::TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) {
	m_int->TraceRay(ray, fMask, pTraceFilter, pTrace);
}
