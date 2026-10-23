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

freecam_t::freecam_t(byte id, mapthing2_t spawn, bool useStartZ) : id(id), cam_pitch(0)
{
	cam_x = INT2FIXED(spawn.x);
	cam_y = INT2FIXED(spawn.y);
	cam_z = useStartZ ? INT2FIXED(spawn.z) : ONFLOORZ;
	cam_angle = MapThingToAngle(spawn.angle);

	cmd.clear();

	AActor* mobj = new AActor(cam_x, cam_y, cam_z, MT_FREECAM);
	mobj->angle = cam_angle;
	mobj->pitch = cam_pitch;
	mobj->prevx = mobj->x;
	mobj->prevy = mobj->y;
	mobj->prevz = mobj->z;
	mobj->prevangle = mobj->angle;
	mobj->prevpitch = mobj->pitch;

	camera = mo = mobj->ptr();
};

bool freecam_t::wipedOnLevelChange()
{
  return (this->mo == nullptr and this->camera == nullptr);
}

void freecam_t::rebuild()
{
	AActor* mobj = new AActor(cam_x, cam_y, cam_z, MT_FREECAM);
	mobj->angle = cam_angle;
	mobj->pitch = cam_pitch;
	mobj->prevx = mobj->x;
	mobj->prevy = mobj->y;
	mobj->prevz = mobj->z;
	mobj->prevangle = mobj->angle;
	mobj->prevpitch = mobj->pitch;

	this->camera = this->mo = mobj->ptr();
}

freecam_t* freecam_t::getfreecam(byte id)
{
	for (auto& fcam : freecams)
	{
		if (fcam.id == id)
			return &fcam;
	}

	return nullptr;
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
