// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2026 by Jess W
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

freecam_t::freecam_t(byte id, mapthing2_t spawn, bool useStartZ) : id(id)
{
	cmd.clear();
	this->build(spawn, useStartZ, true);
};

bool freecam_t::wipedOnLevelChange()
{
	return (this->mo == nullptr and this->camera == nullptr);
}

void freecam_t::build(mapthing2_t spawn, bool useStartZ, bool setSpawnPos)
{
  if (setSpawnPos)
  {
    persist.x = INT2FIXED(spawn.x);
    persist.y = INT2FIXED(spawn.y);
    persist.z = useStartZ ? INT2FIXED(spawn.z) : ONFLOORZ;
    persist.angle = MapThingToAngle(spawn.angle);
    persist.pitch = 0;
  }

	AActor* mobj = new AActor(persist.x, persist.y, persist.z, MT_FREECAM);
	mobj->angle = persist.angle;
	mobj->pitch = persist.pitch;
	mobj->prevx = mobj->x;
	mobj->prevy = mobj->y;
	mobj->prevz = mobj->z;
	mobj->prevangle = mobj->angle;
	mobj->prevpitch = mobj->pitch;

	this->camera = this->mo = mobj->ptr();
}

void freecam_t::persistPosition()
{
	if (this->mo and this->camera)
	{
		persist.x = this->mo->x;
		persist.y = this->mo->y;
		persist.z = this->mo->z;
		persist.angle = this->mo->angle;
		persist.pitch = this->mo->pitch;
	}
}

void freecam_t::reset()
{
	persist.clear();
}

bool freecam_t::needsPosition() const
{
  return (persist.x == 0 and persist.y == 0 and persist.z == 0);
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
