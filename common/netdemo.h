#pragma once

#include <deque>
#include <fstream>

#include "i_net.h"

struct NetDemo
{
    enum netdemo_t
    {
        client_side = 0x01,
        server_side
    };

	[[nodiscard]] bool isRecording() const { return (state == NetDemo::st_recording); }
	[[nodiscard]] bool isPlaying() const { return (state == NetDemo::st_playing); }
	[[nodiscard]] bool isPaused() const { return (state == NetDemo::st_paused); }
	[[nodiscard]] int getSpacing() const { return header.snapshot_spacing; }

	enum netdemo_state_t
	{
		st_stopped,
		st_recording,
		st_playing,
		st_paused
	};

	enum netdemo_message_t
	{
		msg_packet      = 0xAA,
		msg_snapshot,
		msg_map_change,
		msg_eof
	};

	struct message_header_t
	{
		byte        type    { 0 };
		uint32_t    length  { 0 };
		uint32_t    gametic { 0 };
	};

	struct netdemo_index_entry_t
	{
		uint32_t        ticnum  { 0 };
		std::streampos  offset  { 0 };  // offset in the demo file
	};

	static constexpr size_t         HEADER_SIZE = 64;
	static constexpr std::streamoff MESSAGE_HEADER_SIZE = 9;
	static constexpr size_t         INDEX_ENTRY_SIZE = 8;

	static constexpr uint16_t SNAPSHOT_SPACING = 20 * TICRATE;

	struct netdemo_header_id_t
	{
		char        identifier[4]   { 0, 0, 0, 0};  // "ODAD"
		byte        version         { 0 };          // 4, 3, etc...

		bool Read(std::fstream& io_stream);
	};

	// The following exists only for a remote chance of compatibility with old netdemos.
	// At the very least, we want to be able to make sense of the headers, but there's
	// zero guarantee of it working.  In fact, it's likely to not work because the message
	// content is almost certainly different enough to be non-functional with current
	// message body formats.
	struct netdemo_header3_t
	{
		netdemo_header_id_t id              {};     // version 3
		byte        compression             { 0 };  // type of compression used
		uint16_t    snapshot_index_size     { 0 };  // number of snapshots in the index
		uint32_t    snapshot_index_offset   { 0 };  // offset from start of the file for the index
		uint16_t    map_index_size          { 0 };  // number of maps in the mapindex
		uint32_t    map_index_offset        { 0 };  // offset from start of the file for the mapindex
		uint16_t    snapshot_spacing        { 0 };  // number of gametics between indices
		uint32_t    starting_gametic        { 0 };  // the gametic the demo starts at
		uint32_t    ending_gametic          { 0 };  // the last gametic of the demo
		byte        reserved[36]            { 0 };  // for future use

		bool Read(std::fstream& io_stream);
	};

    // Now for the current netdemo version.
	struct netdemo_header4_t
	{
		netdemo_header_id_t id      {};             // version 4
		byte   	    demo_type       {};
		byte        compression     { 0 };          // type of compression used
		uint16_t    snapshot_spacing{ 0 };          // number of gametics between indices
		uint32_t    starting_gametic{ 0 };          // the gametic the demo starts at
		uint32_t    ending_gametic  { 0 };          // the last gametic of the demo
		byte        reserved[47]    { 0 };          // for future use

		bool Read(std::fstream& io_stream);
		void Import(const netdemo_header3_t& oldHeader)
		{
			// we deliberately skip 'id' and 'reserved'.
			demo_type		 = NetDemo::client_side;
			compression      = oldHeader.compression;
			snapshot_spacing = oldHeader.snapshot_spacing;
			starting_gametic = oldHeader.starting_gametic;
			ending_gametic   = oldHeader.ending_gametic;
		}
	};

	bool writeHeader();
	bool readHeader();
	static int LatestDemoVersion(const int version);

	netdemo_state_t state   { st_stopped };
	netdemo_state_t oldstate{ st_stopped };   // used when unpausing
	std::string     filename{ };
	std::fstream    demofp  { };

	std::deque<buf_t>   captured {};

	netdemo_header4_t   header {};

	std::vector<byte>   snapbuf {};
	int                 netdemotic{ 0 };
};
