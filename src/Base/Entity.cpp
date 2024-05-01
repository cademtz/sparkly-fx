#include "Entity.h"
#include "Base.h"
#include "Interfaces.h"
#include "VFunc.h"
#include "Wrappers/Wrappers.h"
#include <SDK/icliententity.h>

// TODO: Make these indexes part of the enum of offset thingies, maybe?

bool CBaseEntity::SetupBones(matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
	size_t index = 16;
	if (Interfaces::engine->GetAppID() == EAppID::CSGO)
		index = 13;

	using SetupBonesFn = bool(__thiscall*)(void*, void*, int, int, float);
	return GetVFunc<SetupBonesFn>(Renderable(), index)(Renderable(), pBoneToWorldOut, nMaxBones, boneMask, currentTime);
}
ClientClass* CBaseEntity::GetClientClass(){
	return ((IClientEntity*)this)->GetClientClass();
}
bool CBaseEntity::IsDormant()
{
	size_t index = 8;
	if (Interfaces::engine->GetAppID() == EAppID::CSGO)
		index = 9;

	using DormantFn = bool (__thiscall*)(void*);
	return GetVFunc<DormantFn>(Networkable(), index)(Networkable());
}
int CBaseEntity::entindex() const
{
	size_t index = 9;
	if (Interfaces::engine->GetAppID() == EAppID::CSGO)
		index = 10;

	using IndexFn = int(__thiscall*)(void*);
	return GetVFunc<IndexFn>(Networkable(), index)(Networkable());
}
