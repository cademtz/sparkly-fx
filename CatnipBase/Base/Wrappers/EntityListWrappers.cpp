#include "EntityListWrappers.h"
#include "SDK/InterfaceVersions/ClientEntityList003.h"

CBaseEntity* IClientEntityListWrapper003::GetClientEntity(int entnum) {
	return (CBaseEntity*)m_int->GetClientEntity(entnum);
}
CBaseEntity* IClientEntityListWrapper003::GetClientEntityFromHandle(CBaseHandle hEnt) {
	return (CBaseEntity*)m_int->GetClientEntityFromHandle(hEnt);
}
int IClientEntityListWrapper003::NumberOfEntities(bool bIncludeNonNetworkable) {
	return m_int->NumberOfEntities(bIncludeNonNetworkable);
}
int IClientEntityListWrapper003::GetHighestEntityIndex(void) {
	return m_int->GetHighestEntityIndex();
}
int IClientEntityListWrapper003::GetMaxEntities() {
	return m_int->GetMaxEntities();
}
