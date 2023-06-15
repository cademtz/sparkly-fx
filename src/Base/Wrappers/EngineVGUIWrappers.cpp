#include "EngineVGUIWrappers.h"
#include <SDK/InterfaceVersions/EngineVGuiSDK.h>
#include <SDK/InterfaceVersions/EngineVGuiTF2.h>
#include <SDK/InterfaceVersions/EngineVGuiCSGO.h>

int CEngineVGUIWrapperSDK::GetOffset(EOffsets Offset)
{
	switch (Offset)
	{
	case Off_Paint:
		return 13;
	}
    return -1;
}

int CEngineVGUIWrapperTF2::GetOffset(EOffsets Offset)
{
	switch (Offset)
	{
	case Off_Paint:
		return 14;
	}
    return -1;
}

int CEngineVGUIWrapperCSGO::GetOffset(EOffsets Offset)
{
	switch (Offset)
	{
	case Off_Paint:
		return 14;
	}
	return -1;
}


bool CEngineVGUIWrapperSDK::IsGameUIVisible() {
	return m_int->IsGameUIVisible();
}
void CEngineVGUIWrapperSDK::Paint(PaintMode_t mode) {
	return m_int->Paint(mode);
}

bool CEngineVGUIWrapperTF2::IsGameUIVisible() {
	return m_int->IsGameUIVisible();
}
void CEngineVGUIWrapperTF2::Paint(PaintMode_t mode) {
	return m_int->Paint(mode);
}

bool CEngineVGUIWrapperCSGO::IsGameUIVisible() {
	return m_int->IsGameUIVisible();
}
void CEngineVGUIWrapperCSGO::Paint(PaintMode_t mode) {
	return m_int->Paint(mode);
}
