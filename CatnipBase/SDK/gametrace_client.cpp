//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

//#include "cbase.h"
#include "gametrace.h"
#include <Base/Interfaces.h>
#include <Base/Entity.h>

// memdbgon must be the last include file in a .cpp file!!!
//#include "tier0/memdbgon.h"

bool CGameTrace::DidHitWorld() const
{
	return m_pEnt == Interfaces::entlist->GetClientEntity(0);
}


bool CGameTrace::DidHitNonWorldEntity() const
{
	return m_pEnt != NULL && !DidHitWorld();
}


int CGameTrace::GetEntityIndex() const
{
	if (m_pEnt)
		return m_pEnt->entindex();
	else
		return -1;
}
