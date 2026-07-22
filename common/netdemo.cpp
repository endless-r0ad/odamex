
#include "netdemo.h"
#include "m_fileio.h"


/**
 * @brief Map demo versions to the latest Odamex version that can read them.
 *
 * @param version Demo version to check.
 * @return Latest Odamex version for that demo in packed format, or 0 if
 *         the demo version is unknown to us.
 */
int NetDemo::LatestDemoVersion(const int version)
{
    switch (version)
    {
    case 4:
        return GAMEVER;
    case 3:
        return MAKEVER(12, 2, 1);
    case 2:
        return MAKEVER(0, 6, 0);
    case 1:
        return MAKEVER(0, 5, 3);
    default:
        return 0;
    }
}

//
// writeHeader()
//
//   Writes the header struct to the netdemo file in little-endian format
//   Assumes that demofp has been opened correctly elsewhere.  Does not close
//   the file.

bool NetDemo::writeHeader()
{
	memcpy(header.id.identifier, "ODAD", 4);
	header.id.version = NETDEMOVER;
	header.compression = 0;
	header.snapshot_spacing = NetDemo::SNAPSHOT_SPACING;

	demofp.seekp(0, std::ios::beg);
	const auto startingPosition = demofp.tellp();

	const bool result = startingPosition >= 0
	                    and M_WriteLE(demofp, header.id.identifier)
	                    and M_WriteLE(demofp, header.id.version)
                        and M_WriteLE(demofp, header.demo_type)
	                    and M_WriteLE(demofp, header.compression)
	                    and M_WriteLE(demofp, header.snapshot_spacing)
	                    and M_WriteLE(demofp, header.starting_gametic)
	                    and M_WriteLE(demofp, header.ending_gametic)
	                    and M_WriteLE(demofp, header.reserved)
	                    and demofp.tellp() - startingPosition == HEADER_SIZE;
	return result;
}



bool NetDemo::netdemo_header_id_t::Read(std::fstream& io_stream)
{
    if (io_stream.good())
    {
        return  M_ReadLE(io_stream, identifier)
            and M_ReadLE(io_stream, version);
    }
    return false;
}

bool NetDemo::netdemo_header3_t::Read(std::fstream& io_stream)
{
    if (io_stream.good())
    {
        return  id.Read(io_stream)
            and M_ReadLE(io_stream, compression)
            and M_ReadLE(io_stream, snapshot_index_size)
            and M_ReadLE(io_stream, snapshot_index_offset)
            and M_ReadLE(io_stream, map_index_size)
            and M_ReadLE(io_stream, map_index_offset)
            and M_ReadLE(io_stream, snapshot_spacing)
            and M_ReadLE(io_stream, starting_gametic)
            and M_ReadLE(io_stream, ending_gametic)
            and M_ReadLE(io_stream, reserved);
    }
    return false;
}

bool NetDemo::netdemo_header4_t::Read(std::fstream& io_stream)
{
    if (io_stream.good())
    {
        return  id.Read(io_stream)
            and M_ReadLE(io_stream, demo_type)
            and M_ReadLE(io_stream, compression)
            and M_ReadLE(io_stream, snapshot_spacing)
            and M_ReadLE(io_stream, starting_gametic)
            and M_ReadLE(io_stream, ending_gametic)
            and M_ReadLE(io_stream, reserved);
    }
    return false;
}

//
// readHeader()
//
//   Reads the header struct from the netdemo file, converting it from
//   little-endian format to whatever the client's architecture uses.  Assumes
//   that demofp has been opened correctly elsewhere.  Does not close the file.

bool NetDemo::readHeader()
{
	demofp.seekg(0, std::ios::beg);
	const auto startingPosition = demofp.tellg();

    netdemo_header_id_t headerId;
    const bool headerIDOk = headerId.Read(demofp);

    if (not (headerIDOk
             and headerId.identifier[0] == 'O'
             and headerId.identifier[1] == 'D'
             and headerId.identifier[2] == 'A'
             and headerId.identifier[3] == 'D'))
    {
        return false;
    }

    header.id = headerId;

    if (header.id.version == NETDEMOVER)
    {
        demofp.seekg(startingPosition, std::ios::beg);

        return header.Read(demofp)
                and demofp.tellg() - startingPosition == HEADER_SIZE;
    }

    if (header.id.version == 3)
    {
        demofp.seekg(startingPosition, std::ios::beg);

        netdemo_header3_t header3;

        if (header3.Read(demofp)
                and demofp.tellg() - startingPosition == HEADER_SIZE)
        {
            // Translate from 3 to NETDEMOVER
            header.Import(header3);
            return true;
        }
    }
	return false;
}
