#pragma once
#include "Wrappers.h"

class IClientEntityList;

class IClientEntityListWrapper003 : public IClientEntityListWrapper
{
	IClientEntityList* m_int;

public:
	IClientEntityListWrapper003(void* EntityList) : m_int((IClientEntityList*)EntityList) { }

	virtual CBaseEntity* GetClientEntity(int entnum);
	virtual CBaseEntity* GetClientEntityFromHandle(CBaseHandle hEnt);
	virtual int					NumberOfEntities(bool bIncludeNonNetworkable);
	virtual int					GetHighestEntityIndex(void);
	virtual int					GetMaxEntities();
};
