#include "Wrappers.h"
#include "Base/Base.h"
#include "Base/Interfaces.h"
#include "EntityWrappers.h"

CEntity* WrapEntity(void* Entity)
{
	switch (Interfaces::engine->GetAppID())
	{
	case AppId_CSGO:
		return nullptr;
	}
	return new CEntityWrapperSDK(Entity);
}
