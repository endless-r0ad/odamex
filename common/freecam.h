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
inline constexpr byte freecam_id = 1; // general use, netdemos and survival
inline byte displaycam_id = 1;

class freecam_t
{
  public:
    freecam_t(byte id, fixed_t x, fixed_t y, fixed_t z, angle_t angle, fixed_t pitch);

    byte id;

    AActor::AActorPtr camera;
    AActor::AActorPtr mo;

    struct ticcmd_t cmd;

    fixed_t cam_x;
    fixed_t cam_y;
    fixed_t cam_z;
    angle_t cam_angle;
    fixed_t cam_pitch;

    void Serialize (FArchive &arc);
    bool wipedOnLevelChange();
    void rebuild(fixed_t x, fixed_t y, fixed_t z, angle_t angle, fixed_t pitch);
    static bool allowAdd();
    static freecam_t& idfreecam(byte id);

};

inline std::list<freecam_t> freecams;
