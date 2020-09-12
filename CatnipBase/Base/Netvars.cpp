#include "Netvars.h"
#include "AltimorNetvar.h"

void Netvars::GetNetvars()
{
	netvar_tree nv;

	m_vecOrigin = nv.get_offset("DT_BaseEntity", "m_vecOrigin");
	m_angRotation = nv.get_offset("DT_BaseEntity", "m_angRotation");
	m_iTeamNum = nv.get_offset("DT_BaseEntity", "m_iTeamNum");
	m_vecMins = nv.get_offset("DT_BaseEntity", "m_Collision", "m_vecMins");
	m_vecMaxs = nv.get_offset("DT_BaseEntity", "m_Collision", "m_vecMaxs");

	m_iHealth = nv.get_offset("DT_BasePlayer", "m_lifeState");
	m_lifeState = nv.get_offset("DT_BasePlayer", "m_lifeState");
	m_vecViewOffset = nv.get_offset("DT_BasePlayer", "localdata", "m_vecViewOffset[0]");
}
