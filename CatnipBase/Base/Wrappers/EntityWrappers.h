#pragma once
#include "Wrappers.h"

class IClientEntity;

class CEntityWrapperSDK : public CEntity
{
	IClientEntity* m_ent;

public:
	CEntityWrapperSDK(void* Entity) : m_ent((IClientEntity*)Entity) { }

	void* Inst() override { return m_ent; }
	virtual bool	SetupBones(matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime);
	virtual ClientClass* GetClientClass();
	virtual bool			IsDormant(void);
	virtual int				entindex(void) const;
};