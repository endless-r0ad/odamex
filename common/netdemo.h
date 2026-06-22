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
//	Base struct for netdemos 
//
//-----------------------------------------------------------------------------

#pragma once

#include "i_net.h"

struct NetDemo
{
    typedef struct
	{
		char		identifier[4];  		// "ODAD"
		byte		version;
		byte    	compression;    		// type of compression used
		uint16_t	snapshot_spacing;		// number of gametics between indices
		uint32_t	starting_gametic;		// the gametic the demo starts at
		uint32_t	ending_gametic;			// the last gametic of the demo
		byte		reserved[48];   		// for future use
	} netdemo_header_t;

    typedef struct
	{
		byte		type;
		uint32_t	length;
		uint32_t	gametic;
	} message_header_t;

    typedef enum
	{
		msg_packet		= 0xAA,
		msg_snapshot,
		msg_map_change,
		msg_eof
	} netdemo_message_t;

    typedef enum
	{
		st_stopped,
		st_recording,
		st_playing,
		st_paused
	} netdemo_state_t;

    [[nodiscard]] bool isRecording() { return (state == NetDemo::st_recording); }
	[[nodiscard]] bool isPlaying() const { return (state == NetDemo::st_playing); }
	[[nodiscard]] bool isPaused() const { return (state == NetDemo::st_paused); }

	static constexpr size_t HEADER_SIZE = 64;
	static constexpr size_t MESSAGE_HEADER_SIZE = 9;

	static constexpr uint16_t SNAPSHOT_SPACING = 20 * TICRATE;

	netdemo_state_t		state = st_stopped;
	netdemo_state_t		oldstate = st_stopped;	// used when unpausing
	std::string			filename = "";
	FILE*				demofp = NULL;
	int					netdemotic = 0;

    netdemo_header_t	header;

	std::vector<byte>	snapbuf;
};
