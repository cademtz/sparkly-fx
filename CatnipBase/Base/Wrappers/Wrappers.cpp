#include "Wrappers.h"
#include "Base/Base.h"
#include "Base/Interfaces.h"
#include "EntityWrappers.h"

CEntityWrapper* WrapEntity(void* Entity)
{
	switch (Interfaces::engine->GetAppID())
	{
	case AppId_CSGO:
		return nullptr;
	}
	return new CEntityWrapperSDK(Entity);
}
