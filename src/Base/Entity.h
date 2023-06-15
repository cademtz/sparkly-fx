#pragma once
#include "Netvars.h"
#include <SDK/vector.h>
#include <SDK/mathlib.h>

#define NV_RET(type, var) return *(type*)(this + var)

class CEntityWrapper;
class IClientEntity;
class ClientClass;
class CBasePlayer;

class CBaseEntity
{
public:
	void* Renderable() const { return (void**)this + 1; }
	void* Networkable() const { return (void**)this + 2; }

	bool			SetupBones(matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime);
	ClientClass*	GetClientClass();
	bool			IsDormant(void);
	int				entindex(void) const;

	inline const Vector& Origin() { NV_RET(Vector, Netvars::m_vecOrigin); }
	inline const QAngle& Rotation() { NV_RET(QAngle, Netvars::m_angRotation); }
	inline const Vector& Mins() { NV_RET(Vector, Netvars::m_vecMins); }
	inline const Vector& Maxs() { NV_RET(Vector, Netvars::m_vecMaxs); }
	inline int Team() { NV_RET(int, Netvars::m_iTeamNum); }

	CBasePlayer* ToPlayer() { return (CBasePlayer*)this; }
};

class CBasePlayer : public CBaseEntity
{
public:
	inline const Vector& ViewOffset() { NV_RET(Vector, Netvars::m_vecViewOffset); }
	inline const int LifeState() { NV_RET(int, Netvars::m_lifeState); }
	inline const int Health() { NV_RET(int, Netvars::m_iHealth); }
};
