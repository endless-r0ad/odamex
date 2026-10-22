// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2006-2026 by The Odamex Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//   Controllable freecam object for various use cases
//
//-----------------------------------------------------------------------------

#include "freecam.h"
#include <p_local.h>

freecam_t::freecam_t(byte id, fixed_t x, fixed_t y, fixed_t z, angle_t angle, fixed_t pitch)
    : id(id), cam_x(x), cam_y(y), cam_z(z), cam_angle(angle), cam_pitch(pitch)
{
	cmd.clear();
	AActor* mobj = new AActor(x, y, z, MT_FREECAM);
	mobj->angle = angle;
	mobj->pitch = pitch;
	mobj->prevx = mobj->x;
	mobj->prevy = mobj->y;
	mobj->prevz = mobj->z;
	mobj->prevangle = mobj->angle;
	mobj->prevpitch = mobj->pitch;

	camera = mo = mobj->ptr();

	/*uint32_t max_u32 = 4294967295;

	P_SetThingId(mobj, max_u32 - freecams.size());

	mobj->LinkToWorld();

	if (mobj->subsector)
	{
		mobj->floorz = P_FloorHeight(mobj);
		mobj->ceilingz = P_CeilingHeight(mobj);
		mobj->dropoffz = mobj->floorz;
		mobj->floorsector = mobj->subsector->sector;
	}

	P_SetMobjState(mobj, mobj->info->spawnstate);*/
};

bool freecam_t::wipedOnLevelChange()
{
  return (this->mo == nullptr and this->camera == nullptr);
}

void freecam_t::rebuild(fixed_t x, fixed_t y, fixed_t z, angle_t angle, fixed_t pitch)
{
	AActor* mobj = new AActor(x, y, z, MT_FREECAM);
	mobj->angle = angle;
	mobj->pitch = pitch;
	mobj->prevx = mobj->x;
	mobj->prevy = mobj->y;
	mobj->prevz = mobj->z;
	mobj->prevangle = mobj->angle;
	mobj->prevpitch = mobj->pitch;

	this->camera = this->mo = mobj->ptr();

	/*uint32_t max_u32 = 4294967295;

	P_SetThingId(mobj, max_u32 - freecams.size());

	mobj->LinkToWorld();

	if (mobj->subsector)
	{
		mobj->floorz = P_FloorHeight(mobj);
		mobj->ceilingz = P_CeilingHeight(mobj);
		mobj->dropoffz = mobj->floorz;
		mobj->floorsector = mobj->subsector->sector;
	}

	P_SetMobjState(mobj, mobj->info->spawnstate);*/
}

freecam_t& freecam_t::idfreecam(byte id)
{
	for (auto& fcam : freecams)
	{
		if (fcam.id == id)
			return fcam;
	}

	return *new freecam_t(0, 0, 0, 0, 0, 0);
}

void freecam_t::Serialize (FArchive &arc)
{
	if (arc.IsStoring ())
	{ // saving to archive
		arc << id
			<< cmd
			<< cam_x
			<< cam_y
			<< cam_z
			<< cam_angle
			<< cam_pitch;
	}
	else
	{ // Restoring from archive
		UserInfo dummyuserinfo;

		arc >> id
			>> cmd
			>> cam_x
			>> cam_y
			>> cam_z
			>> cam_angle
			>> cam_pitch;

		camera = mo;
	}
}
