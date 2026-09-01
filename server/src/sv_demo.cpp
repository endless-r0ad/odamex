// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2006 by Randy Heit (ZDoom).
// Copyright (C) 2000-2006 by Sergey Makovkin (CSDoom .62).
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
//	Functions for recording and playing back recordings of network games
//
//-----------------------------------------------------------------------------


#include "odamex.h"

#include "sv_main.h"
#include "p_ctf.h"
#include "d_player.h"
#include "m_argv.h"
#include "c_console.h"
#include "m_fileio.h"
#include "sv_demo.h"
#include "p_saveg.h"
#include "r_main.h"
#include "st_stuff.h"
#include "p_mobj.h"
#include "clc_message.h"
#include "svc_message.h"
#include "g_gametype.h"

#include "PacketHeaderType.h"

EXTERN_CVAR(sv_maxclients)
EXTERN_CVAR(sv_maxplayers)
EXTERN_CVAR(sv_hostname)

static player_t nullplayer;

// Want to press your luck loading previous-versioned netdemos?  Press this button!  Don't get a whammy!
constexpr bool TRY_LOADING_OLD_NETDEMOS = false;

extern OResFiles wadfiles;

extern bool hasReceivedFullUpdate;


ServerNetDemo::~ServerNetDemo()
{
	cleanUp();
}

void ServerNetDemo::reset()
{
	cleanUp();

	filename = "";
	header = netdemo_header4_t{};
	captured.clear();
}

//
// cleanUp
//
//   Attempts to close any open files and generally exit gracefully.
//

void ServerNetDemo::cleanUp()
{
	if (isRecording())
	{
		stopRecording();	// Try to write any unwritten data
	}

	// close all files
	demofp.close();

	state = oldstate = NetDemo::st_stopped;
	netdemotic = last_map_tic = 0;
}

/**
 * Error handler.
 *
 * Generic error handler for netdemo issues.
 *
 * @param message Error message.
 */
void ServerNetDemo::error(const std::string &message)
{
	cleanUp();
	PrintFmt(PRINT_HIGH, "{}\n", message);
}


//
// startRecording()
//
//   Creates the netdemo file with the specified filename.  A temporary
//   header is written which will be overwritten with the proper information
//   in stopRecording().

bool ServerNetDemo::startRecording(const std::string &filename)
{
	this->filename = filename;

	if (isPlaying() || isPaused())
	{
		error("Cannot record a netdemo while not connected to a server.");
		return false;
	}

	// Already recording so just ignore the command
	if (isRecording())
		return true;

	demofp.close();

	demofp = std::fstream(filename,
	                      std::ios::out |
	                      std::ios::binary |
	                      std::ios::trunc);
	if (not demofp.good())
	{
		error("Unable to create netdemo file " + filename + ".");
		//I_Warning("Unable to create netdemo file {}", filename);
		return false;
	}

	header = netdemo_header4_t{};
	header.starting_gametic = gametic;
	header.demo_type = NetDemo::server_side;

	// Note: The header is not finalized at this point.  Write it anyway to
	// reserve space in the output file for it and overwrite it later.
	if (!writeHeader())
	{
		error("Unable to write netdemo header.");
		return false;
	}

	state = NetDemo::st_recording;
	PrintFmt(PRINT_HIGH, "Recording netdemo {}.\n", filename);

	if (isRecording()) // (connected)
	{
		// write a simulation of the connection sequence since the server
		// has already sent it to the client and it wasn't captured
		static buf_t tempbuf(NETDEMO_STARTUP_PACKET_SIZE);

		// Fake the launcher query response
		SZ_Clear(&tempbuf);
		writeLauncherSequence(&tempbuf);
		capture(&tempbuf);
		writeMessages(true);

		// Fake the server's side of the connection sequence
		SZ_Clear(&tempbuf);
		writeConnectionSequence(&tempbuf);
		capture(&tempbuf);
		writeMessages(true);

		SZ_Clear(&tempbuf);
		MSG_WriteSVCBuffer(&tempbuf, odaproto::clc::NetDemoLoadSnap());
		capture(&tempbuf);
		writeMessages(true);

		// Record any additional messages (usually a full update if auto-recording))
		// Do not write this message immediately because it needs to be written after
		// the map snapshot.
		capture(&net_message);
	}

	return true;
}


//
// stopRecording()
//
//   Writes the netdemo index to file and rewrites the netdemo header before
//   closing the netdemo file.

bool ServerNetDemo::stopRecording()
{
	if (!isRecording())
	{
		return false;
	}
	state = NetDemo::st_stopped;

	// write any remaining messages that have been captured
	writeMessages();

	// write the end-of-demo marker - header + size
	byte stopdata[2] = {clc_netdemostop, 0};
	writeChunk(&stopdata[0], sizeof(stopdata), NetDemo::msg_eof);

	// write the number of the last gametic in the recording
	header.ending_gametic = gametic;

	demofp.flush();

	// rewrite the header for ending_gametic
	if (!writeHeader())
	{
		error("Unable to write updated netdemo header.");
		return false;
	}

	demofp.close();

	PrintFmt(PRINT_HIGH, "Demo recording has stopped.\n");
	reset();
	return true;
}


//
// writeLocalCmd()
//
//   Generates a message indicating the current position and angle of the
//   consoleplayer, taking the place of ticcmds.
void ServerNetDemo::writeLocalCmd(buf_t *netbuffer) const
{
	// Record the local player's data
	player_t& player = nullplayer;
	if (not player.mo)
		return;

	//MSG_WriteSVCBuffer(netbuffer, CLC_NetdemoCap(player, localcmds[gametic % MAXSAVETICS], ::messenger));
}


void ServerNetDemo::writeChunk(const byte *data, size_t size, netdemo_message_t type)
{
	message_header_t msgheader;

	msgheader.type      = static_cast<byte>(type);
	msgheader.length    = size;
	msgheader.gametic   = gametic;

	const auto startingPosition = demofp.tellp();
	const bool headerResult = startingPosition >= 0
	                            and M_WriteLE(demofp, msgheader.type)
	                            and M_WriteLE(demofp, msgheader.length)
	                            and M_WriteLE(demofp, msgheader.gametic)
	                            and demofp.tellp() - startingPosition == MESSAGE_HEADER_SIZE;

	if (headerResult)
	{
		const auto dataStartPosition = demofp.tellp();
		demofp.write(reinterpret_cast<const char*>(data), size);
		if (demofp.tellp() - dataStartPosition != size)
		{
			error("Unable to write netdemo message chunk\n");
		}
	}
}


//
// atSnapshotInterval()
//
//    Returns true if it is the appropriate time to write a snapshot
//
bool ServerNetDemo::atSnapshotInterval()
{
	if (!isRecording() || last_map_tic == 0 || gamestate != GS_LEVEL)
		return false;

	if (gametic == last_map_tic)
		return false;

	return ((gametic - last_map_tic) % header.snapshot_spacing == 0);
}


void ServerNetDemo::ticker()
{
	netdemotic++;
}

//
// writeMessages()
//
//   Writes the packets received from the server and captures local player
//   input and writes to the netdemo file.
//

void ServerNetDemo::writeMessages(bool isNetdemoStartup)
{
	if (!isRecording())
		return;

	if (atSnapshotInterval())
	{
		writeSnapshotData(snapbuf);
		writeChunk(snapbuf.data(), snapbuf.size(), NetDemo::msg_snapshot);
	}

	auto output_buf = std::make_unique<byte[]>(
    captured.size() * (isNetdemoStartup ? NETDEMO_STARTUP_PACKET_SIZE : MAX_UDP_PACKET)
  );

	uint32_t output_len = 0;
	while (!captured.empty())
	{
		buf_t netbuf(std::move(captured.front()));
		uint32_t len = netbuf.BytesLeftToRead();

		byte *chunk = netbuf.ReadChunk(len);

    if (!chunk)
     break;

		memcpy(&output_buf[output_len], chunk, len);
		output_len += len;

		captured.pop_front();
	}

	writeChunk(output_buf.get(), output_len, NetDemo::msg_packet);
}


//
// writeLauncherSequence()
//
//   Emulates the sequence of messages the server sends a launcher program or
//   the client when a client first contacts a server to initiate a connection.
//   As much of this data is parsed and ignored by a connecting client, a good
//   deal of the data written to netbuffer is simply place holding data and not
//   accurate.
//

void ServerNetDemo::writeLauncherSequence(buf_t *netbuffer)
{
	// Server sends launcher info
	MSG_WriteLong   (netbuffer, PROTO_CHALLENGE);
	MSG_WriteLong   (netbuffer, 0);     // server_token

	// get sv_hostname and write it
	MSG_WriteString (netbuffer, sv_hostname.cstring());

	int playersingame = std::count_if(players.cbegin(), players.cend(), [](const auto& player){ return player.ingame(); });
	MSG_WriteByte   (netbuffer, playersingame);
	MSG_WriteByte   (netbuffer, 0);             // sv_maxclients
	MSG_WriteString (netbuffer, level.mapname.c_str());

	// names of all the wadfiles on the server
	size_t numwads = wadfiles.size();
	if (numwads > 0xff)
	    numwads = 0xff;
	MSG_WriteByte   (netbuffer, numwads - 1);

	for (size_t n = 1; n < numwads; n++)
	{
		// Don't use absolute paths, as they present a security risk.
		MSG_WriteString(netbuffer, ::wadfiles[n].getBasename().c_str());
	}

	MSG_WriteBool   (netbuffer, 0);     // deathmatch?
	MSG_WriteByte   (netbuffer, 0);     // sv_skill
	MSG_WriteBool   (netbuffer, (sv_gametype == GM_TEAMDM));
	MSG_WriteBool   (netbuffer, (sv_gametype == GM_CTF));

	for (const auto& player : players)
	{
		// Notes: client just ignores this data but still expects to parse it
		if (player.ingame())
		{
			MSG_WriteString(netbuffer, ""); // player's netname
			MSG_WriteShort(netbuffer, 0); // player's fragcount
			MSG_WriteLong(netbuffer, 0); // player's ping
			MSG_WriteByte(netbuffer, 0); // player's team
		}
	}

	// MD5 hash sums for all the wadfiles on the server
	for (size_t n = 1; n < numwads; n++)
		MSG_WriteString(netbuffer, ::wadfiles[n].getMD5().getHexCStr());

	MSG_WriteString (netbuffer, "");    // sv_website.cstring()

	if (G_IsTeamGame())
	{
		MSG_WriteLong   (netbuffer, 0);     // sv_scorelimit
		for (size_t n = 0; n < NUMTEAMS; n++)
		{
			MSG_WriteBool   (netbuffer, false);
		}
	}

	MSG_WriteShort  (netbuffer, VERSION);

	// Note: these are ignored by clients when the client connects anyway
	// so they don't need real data
	MSG_WriteString (netbuffer, "");    // sv_email.cstring()

	MSG_WriteShort  (netbuffer, 0);     // sv_timelimit
	MSG_WriteShort  (netbuffer, 0);     // timeleft before end of level
	MSG_WriteShort  (netbuffer, 0);     // sv_fraglimit

	MSG_WriteBool   (netbuffer, false); // sv_itemrespawn
	MSG_WriteBool   (netbuffer, false); // sv_weaponstay
	MSG_WriteBool   (netbuffer, false); // sv_friendlyfire
	MSG_WriteBool   (netbuffer, false); // sv_allowexit
	MSG_WriteBool   (netbuffer, false); // sv_infiniteammo
	MSG_WriteBool   (netbuffer, false); // sv_nomonsters
	MSG_WriteBool   (netbuffer, false); // sv_monstersrespawn
	MSG_WriteBool   (netbuffer, false); // sv_fastmonsters
	MSG_WriteBool   (netbuffer, false); // sv_allowjump
	MSG_WriteBool   (netbuffer, false); // sv_freelook
	MSG_WriteBool   (netbuffer, false); // sv_waddownload -- removed
	MSG_WriteBool   (netbuffer, false); // sv_emptyreset
	MSG_WriteBool   (netbuffer, false); // sv_cleanmaps -- removed
	MSG_WriteBool   (netbuffer, false); // sv_fragexitswitch

	for (const auto& player : players)
	{
		if (player.ingame())
		{
			MSG_WriteShort(netbuffer, player.killcount);
			MSG_WriteShort(netbuffer, player.deathcount);

			int timeingame = (time(NULL) - player.JoinTime) / 60;
			if (timeingame < 0)
				timeingame = 0;
			MSG_WriteShort(netbuffer, timeingame);
		}
	}

	MSG_WriteLong(netbuffer, static_cast<uint32_t>(0x01020304));
	MSG_WriteShort(netbuffer, sv_maxplayers);

	for (const auto& player : players)
	{
		if (player.ingame())
			MSG_WriteBool(netbuffer, player.spectator);
	}

	MSG_WriteLong   (netbuffer, static_cast<uint32_t>(0x01020305));
	MSG_WriteShort  (netbuffer, 0); // join_passowrd

	MSG_WriteLong   (netbuffer, GAMEVER);

	// TODO: handle patch files
	MSG_WriteByte   (netbuffer, 0);  // patchfiles.size()
//  MSG_WriteByte   (netbuffer, patchfiles.size());

//  for (size_t n = 0; n < patchfiles.size(); n++)
//      MSG_WriteString(netbuffer, patchfiles[n].c_str());
}

//
// writeConnectionSequence()
//
//   Emulates the sequence of messages that the server sends to a client in
//   the packet with sequence number 0 and writes them to netbuffer.
//
void ServerNetDemo::writeConnectionSequence(buf_t *netbuffer)
{
	PacketHeaderType header {0};

	header.Pack(*netbuffer);

	const player_t& first_player = players.front();

	// Server sends our player id and digest
	MSG_WriteSVCBuffer(netbuffer, SVC_ConsolePlayer(first_player, first_player.client.digest));

	// our userinfo
	MSG_WriteSVCBuffer(netbuffer, SVC_UserInfo(first_player, 1));

	// Server sends its settings
	cvar_t *var = GetFirstCvar();
	while (var)
	{
		if (var->flags() & CVAR_SERVERINFO)
		{
			MSG_WriteSVCBuffer(netbuffer, SVC_ServerSettings(*var));
		}
		var = var->GetNext();
	}

	// Server tells everyone if we're a spectator
	MSG_WriteSVCBuffer(netbuffer, SVC_PlayerMembers(first_player, SVC_PM_SPECTATOR));

	// Server sends wads & map name
	MSG_WriteSVCBuffer(netbuffer, SVC_LoadMap(::wadfiles, ::patchfiles, ::level.mapname.c_str(), 0));

	// Server spawns the player
	MSG_WriteSVCBuffer(netbuffer, SVC_SpawnPlayer(first_player, gametic));
}


//
// calculateTotalTime()
//
//   Returns the total length of the demo in seconds
//
int ServerNetDemo::calculateTotalTime() const
{
	if (!isPlaying() && !isPaused())
		return 0;

	return ((header.ending_gametic - header.starting_gametic) / TICRATE);
}


//
// calculateTimeElapsed()
//
//   Returns the number of seconds since the demo started playing
//
int ServerNetDemo::calculateTimeElapsed() const
{
	if (!isPlaying() && !isPaused())
		return 0;

	int elapsed = netdemotic / TICRATE;
	int totaltime = calculateTotalTime();

	if (elapsed > totaltime)
		return totaltime;

	return elapsed;
}


void ServerNetDemo::writeMapChange()
{
	if (isRecording())
	{
		writeSnapshotData(snapbuf);
		writeChunk(snapbuf.data(), snapbuf.size(), NetDemo::msg_map_change);
		last_map_tic = gametic;
	}
}

void ServerNetDemo::writeIntermission()
{
	if (isRecording() && gamestate == GS_INTERMISSION)
	{
		writeSnapshotData(snapbuf);
		writeChunk(snapbuf.data(), snapbuf.size(), NetDemo::msg_snapshot);
	}
}

//
// writeSnapshotData()
//
//   Write the entire state of the game to netbuffer.  Called by
//   writeSnapshot() and used to simulate SV_ClientFullUpdate() when
//   writing the connection sequence at the start of a netdemo.
//
void ServerNetDemo::writeSnapshotData(std::vector<byte>& buf)
{
	G_SnapshotLevel(true);

	FLZOMemFile memfile;
	memfile.Open();         // open for writing

	FArchive arc(memfile);

	// Save the server cvars
	byte vars[4096], *vars_p;
	vars_p = vars;

	cvar_t::C_WriteCVars(&vars_p, CVAR_SERVERINFO, 4096);
	arc.WriteCount(vars_p - vars);
	arc.Write(vars, vars_p - vars);

	// write wad info
	arc << static_cast<byte>(wadfiles.size() - 1);
	for (size_t i = 1; i < wadfiles.size(); i++)
	{
		arc << D_CleanseFileName(::wadfiles[i].getBasename()).c_str();
		arc << ::wadfiles[i].getMD5().getHexCStr();
	}

	arc << static_cast<byte>(patchfiles.size());
	for (const auto& file : patchfiles)
	{
		arc << D_CleanseFileName(file.getBasename()).c_str();
		arc << file.getMD5().getHexCStr();
	}

	// write map info
	arc << level.mapname.c_str();
	arc << static_cast<byte>(gamestate == GS_INTERMISSION);

	G_SerializeSnapshots(arc);
	P_SerializeRNGState(arc);
	P_SerializeACSDefereds(arc);
	P_SerializeHorde(arc);
	SerializeTeamFlagData(arc);

	// Save team points
	for (int i = 0; i < NUMTEAMS; i++)
		arc << GetTeamInfo(static_cast<team_t>(i))->Points;

	arc << level.time;

	for (int i = 0; i < NUM_WORLDVARS; i++)
	{
		arc << ACS_WorldVars[i];
		ACSWorldGlobalArray worldarr = ACS_WorldArrays[i];
		arc << worldarr.size();
		for (const auto& [key, val] : worldarr)
		{
			arc << key;
			arc << val;
		}
	}


	for (int i = 0; i < NUM_GLOBALVARS; i++)
	{
		arc << ACS_GlobalVars[i];
		ACSWorldGlobalArray globalarr = ACS_GlobalArrays[i];
		arc << globalarr.size();
		for (const auto& [key, val] : globalarr)
		{
			arc << key;
			arc << val;
		}
	}

  P_SerializeSprees(arc);

	byte check = 0x1d;
	arc << check;          // consistancy marker

	arc.Close();

	// Resize the snapshot buffer to hold our snapshot size.
	buf.resize(memfile.Length());
	memfile.WriteToBuffer(buf.data(), buf.size());

	if (level.info->snapshot != NULL)
	{
		delete level.info->snapshot;
		level.info->snapshot = NULL;
	}
}


VERSION_CONTROL (sv_demo_cpp, "$Id$")
