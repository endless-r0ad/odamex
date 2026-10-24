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
#pragma once

#include "odamex.h"
#include <list>

inline constexpr byte nullcam_id = 0;
inline constexpr byte firstcam_id = 1;

// the freecam currently being displayed and controlled
inline byte freecam_id = 0;

struct save_pos_t
{
	int tic{0};
	fixed_t x{0};
	fixed_t y{0};
	fixed_t z{0};
	fixed_t angle{0};
	fixed_t pitch{0};

	void clear() { tic = x = y = z = angle = pitch = 0; }
};

class freecam_t
{
public:
	freecam_t(byte id, mapthing2_t spawn, bool useStartZ);

	byte id;

	AActor::AActorPtr camera;
	AActor::AActorPtr mo;

	struct ticcmd_t cmd;
	struct save_pos_t persist;

	bool wipedOnLevelChange();
	void rebuild();
	void savePosition();
	static freecam_t* getfreecam(byte id);
};

inline std::list<freecam_t> freecams;
