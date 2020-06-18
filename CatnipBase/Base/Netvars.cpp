#include "Netvars.h"
#include "AltimorNetvar.h"

void Netvars::GetNetvars()
{
	netvar_tree nv;
	baseentity.m_vecOrigin = nv.get_offset("DT_BaseEntity", "m_vecOrigin");
	baseentity.m_angRotation = nv.get_offset("DT_BaseEntity", "m_angRotation");
	baseentity.m_iTeamNum = nv.get_offset("DT_BaseEntity", "m_iTeamNum");
}
