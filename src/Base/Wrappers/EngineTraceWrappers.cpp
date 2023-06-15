#include "EngineTraceWrappers.h"
#include <SDK/InterfaceVersions/EngineTrace003.h>
#include <SDK/InterfaceVersions/EngineTrace004.h>

int IEngineTrace003Wrapper::GetPointContents(const Vector& vecAbsPosition, IHandleEntity** ppEntity) {
    return m_int->GetPointContents(vecAbsPosition, ppEntity);
}
void IEngineTrace003Wrapper::ClipRayToEntity(const Ray_t& ray, unsigned int fMask, IHandleEntity* pEnt, trace_t* pTrace) {
	return m_int->ClipRayToEntity(ray, fMask, pEnt, pTrace);
}
void IEngineTrace003Wrapper::TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) {
	m_int->TraceRay(ray, fMask, pTraceFilter, pTrace);
}
bool IEngineTrace003Wrapper::PointOutsideWorld(const Vector& ptTest) {
	return m_int->PointOutsideWorld(ptTest);
}

int IEngineTrace004Wrapper::GetPointContents(const Vector& vecAbsPosition, IHandleEntity** ppEntity) {
    return m_int->GetPointContents(vecAbsPosition, ppEntity);
}
void IEngineTrace004Wrapper::ClipRayToEntity(const Ray_t& ray, unsigned int fMask, IHandleEntity* pEnt, trace_t* pTrace) {
	return m_int->ClipRayToEntity(ray, fMask, pEnt, pTrace);
}
void IEngineTrace004Wrapper::TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) {
	m_int->TraceRay(ray, fMask, pTraceFilter, pTrace);
}
bool IEngineTrace004Wrapper::PointOutsideWorld(const Vector& ptTest) {
	assert(false);
	return false;
}
