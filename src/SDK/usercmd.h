//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#if !defined( USERCMD_H )
#define USERCMD_H
#ifdef _WIN32
#pragma once
#endif

#include "vector.h"
//#include "utlvector.h"
#include "mathlib.h"
//#include "imovehelper.h"
#include "checksum_crc.h"
#include <Base/Interfaces.h>

class bf_read;
class bf_write;

class CEntityGroundContact
{
public:
	int					entindex;
	float				minheight;
	float				maxheight;
};

class CUserCmd
{
public:
	CUserCmd()
	{
		Reset();
	}

	virtual ~CUserCmd() { };

	void Reset()
	{
		command_number() = 0;
		tick_count() = 0;
		viewangles().Init();
		forwardmove() = 0.0f;
		sidemove() = 0.0f;
		upmove() = 0.0f;
		buttons() = 0;
		impulse() = 0;
		weaponselect() = 0;
		weaponsubtype() = 0;
		random_seed() = 0;
		mousedx() = 0;
		mousedy() = 0;

		hasbeenpredicted() = false;
#if defined( HL2_DLL ) || defined( HL2_CLIENT_DLL )
		entitygroundcontact.RemoveAll();
#endif
	}

	// Commented out. Doesn't account for CSGO (yet)
	/*CUserCmd& operator =(const CUserCmd& src)
	{
		if (this == &src)
			return *this;

		command_number = src.command_number;
		tick_count = src.tick_count;
		viewangles = src.viewangles;
		forwardmove = src.forwardmove;
		sidemove = src.sidemove;
		upmove = src.upmove;
		buttons = src.buttons;
		impulse = src.impulse;
		weaponselect = src.weaponselect;
		weaponsubtype = src.weaponsubtype;
		random_seed = src.random_seed;
		mousedx = src.mousedx;
		mousedy = src.mousedy;

		hasbeenpredicted = src.hasbeenpredicted;

#if defined( HL2_DLL ) || defined( HL2_CLIENT_DLL )
		entitygroundcontact = src.entitygroundcontact;
#endif

		return *this;
	}

	CUserCmd(const CUserCmd& src)
	{
		*this = src;
	}

	CRC32_t GetChecksum(void) const
	{
		CRC32_t crc;

		CRC32_Init(&crc);
		CRC32_ProcessBuffer(&crc, &command_number, sizeof(command_number));
		CRC32_ProcessBuffer(&crc, &tick_count, sizeof(tick_count));
		CRC32_ProcessBuffer(&crc, &viewangles, sizeof(viewangles));
		CRC32_ProcessBuffer(&crc, &forwardmove, sizeof(forwardmove));
		CRC32_ProcessBuffer(&crc, &sidemove, sizeof(sidemove));
		CRC32_ProcessBuffer(&crc, &upmove, sizeof(upmove));
		CRC32_ProcessBuffer(&crc, &buttons, sizeof(buttons));
		CRC32_ProcessBuffer(&crc, &impulse, sizeof(impulse));
		CRC32_ProcessBuffer(&crc, &weaponselect, sizeof(weaponselect));
		CRC32_ProcessBuffer(&crc, &weaponsubtype, sizeof(weaponsubtype));
		CRC32_ProcessBuffer(&crc, &random_seed, sizeof(random_seed));
		CRC32_ProcessBuffer(&crc, &mousedx, sizeof(mousedx));
		CRC32_ProcessBuffer(&crc, &mousedy, sizeof(mousedy));
		CRC32_Final(&crc);

		return crc;
	}*/

	// Allow command, but negate gameplay-affecting values
	void MakeInert(void)
	{
		viewangles() = vec3_angle;
		forwardmove() = 0.f;
		sidemove() = 0.f;
		upmove() = 0.f;
		buttons() = 0;
		impulse() = 0;
	}

private:

	// For matching server and client commands for debugging
	int		_command_number;

	// the tick the client created this command
	int		_tick_count;

	// Player instantaneous view angles.
	QAngle	_viewangles;

	struct common_vals_struct
	{
		// Intended velocities
		//	forward velocity.
		float	forwardmove;
		//  sideways velocity.
		float	sidemove;
		//  upward velocity.
		float	upmove;
		// Attack button states
		int		buttons;
		// Impulse command issued.
		byte    impulse;
		// Current weapon id
		int		weaponselect;
		int		weaponsubtype;

		int		random_seed;	// For shared random functions

		short	mousedx;		// mouse accum in x from create move
		short	mousedy;		// mouse accum in y from create move

		// Client only, tracks whether we've predicted this command at least once
		bool	hasbeenpredicted;
	};
	union
	{
		struct {
			common_vals_struct values;
		} game_other;

		struct
		{
			Vector aimdirection;
			common_vals_struct values;
		} game_csgo;
	};

	common_vals_struct* GetCommonValues()
	{
		sizeof(game_other);
		sizeof(game_csgo);
		if (Interfaces::engine->GetAppID() == AppId_CSGO)
			return &game_csgo.values;
		return &game_other.values;
	}

	// Back channel to communicate IK state
#if defined( HL2_DLL ) || defined( HL2_CLIENT_DLL )
	CUtlVector< CEntityGroundContact > entitygroundcontact;
#endif

public:
	// TODO: Just make a wrapper class...
	// People might screw up game memory by copying around this improperly-sized struct
	int& command_number() { return _command_number; }
	int& tick_count() { return _tick_count; }
	QAngle& viewangles() { return _viewangles; }
	float& forwardmove() { return GetCommonValues()->forwardmove; }
	float& sidemove() { return GetCommonValues()->sidemove; }
	float& upmove() { return GetCommonValues()->upmove; }
	int& buttons() { return GetCommonValues()->buttons; }
	byte& impulse() { return GetCommonValues()->impulse; }
	int& weaponselect() { return GetCommonValues()->weaponselect; }
	int& weaponsubtype() { return GetCommonValues()->weaponsubtype; }
	int& random_seed() { return GetCommonValues()->random_seed; }
	short& mousedx() { return GetCommonValues()->mousedx; }
	short& mousedy() { return GetCommonValues()->mousedy; }
	bool& hasbeenpredicted() { return GetCommonValues()->hasbeenpredicted; }
};

void ReadUsercmd(bf_read* buf, CUserCmd* move, CUserCmd* from);
void WriteUsercmd(bf_write* buf, const CUserCmd* to, const CUserCmd* from);

#endif // USERCMD_H
