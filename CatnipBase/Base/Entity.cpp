#include "Entity.h"
#include "Wrappers/Wrappers.h"

CBaseEntity::CBaseEntity(IClientEntity* Entity)
	: m_ent(WrapEntity(Entity)) { }

CBaseEntity::~CBaseEntity() {
	delete m_ent;
}

void* CBaseEntity::Inst() {
    return m_ent->Inst();
}
bool CBaseEntity::SetupBones(matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) {
	return m_ent->SetupBones(pBoneToWorldOut, nMaxBones, boneMask, currentTime);
}
ClientClass* CBaseEntity::GetClientClass() {
	return m_ent->GetClientClass();
}
bool CBaseEntity::IsDormant(void) {
	return m_ent->IsDormant();
}
int CBaseEntity::entindex(void) const {
	return m_ent->entindex();
}
