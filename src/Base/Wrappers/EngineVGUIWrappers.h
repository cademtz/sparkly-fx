#pragma once
#include "Wrappers.h"

class IEngineVGuiInternal;
class CEngineVGuiTF2;
class CEngineVGuiCSGO;

class CEngineVGUIWrapperSDK : public CEngineVGUIWrapper
{
	IEngineVGuiInternal* m_int;

public:
	CEngineVGUIWrapperSDK(void* Int) : m_int((IEngineVGuiInternal*)Int) { }

	void* Inst() override { return m_int; }
	int GetOffset(EOffsets Offset) override;

	virtual bool IsGameUIVisible();
	virtual void Paint(PaintMode_t mode);
};

class CEngineVGUIWrapperTF2 : public CEngineVGUIWrapper
{
	CEngineVGuiTF2* m_int;

public:
	CEngineVGUIWrapperTF2(void* Int) : m_int((CEngineVGuiTF2*)Int) { }

	void* Inst() override { return m_int; }
	int GetOffset(EOffsets Offset) override;

	virtual bool IsGameUIVisible();
	virtual void Paint(PaintMode_t mode);
};

class CEngineVGUIWrapperCSGO : public CEngineVGUIWrapper
{
	CEngineVGuiCSGO* m_int;

public:
	CEngineVGUIWrapperCSGO(void* Int) : m_int((CEngineVGuiCSGO*)Int) { }

	void* Inst() override { return m_int; };
	int GetOffset(EOffsets Offset) override;

	virtual bool IsGameUIVisible();
	virtual void Paint(PaintMode_t mode);
};