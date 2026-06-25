#pragma once

#include "i_net.h"
#include <deque>
#include "netdemo.h"

class ClientNetDemo : public NetDemo
{
public:
	ClientNetDemo();
	~ClientNetDemo();
	ClientNetDemo(const ClientNetDemo &rhs);
	ClientNetDemo& operator=(const ClientNetDemo &rhs);

	bool startPlaying(const std::string &filename);
	bool startRecording(const std::string &filename);
	bool stopPlaying();
	bool stopRecording();
	bool pause();
	bool resume();

	void writeMessages();
	void readMessages(buf_t* netbuffer);
	void capture(const buf_t* netbuffer);
	void capture(const std::basic_string<byte>& buffer);
	void writeMapChange();
	void writeIntermission();

	[[nodiscard]] int getSpacing() const { return header.snapshot_spacing; }

	void nextTic();
	void nextSnapshot();
	void prevSnapshot();
	void nextMap();
	void prevMap();

	void ticker();
	[[nodiscard]] int calculateTimeElapsed() const;
	[[nodiscard]] int calculateTotalTime() const;
	[[nodiscard]] const std::vector<int> getMapChangeTimes() const;
	[[nodiscard]] const std::string &getFileName() const { return filename; }

private:
	typedef struct
	{
		uint32_t	ticnum;
		uint32_t	offset;			// offset in the demo file
	} netdemo_index_entry_t;

	void cleanUp();
	void copy(ClientNetDemo &to, const ClientNetDemo &from);
	void error(const std::string &message);
	void fatalError(const std::string &message);
	void reset();

	[[nodiscard]] const netdemo_index_entry_t *snapshotLookup(int ticnum) const;
	void writeLauncherSequence(buf_t *netbuffer);
	void writeConnectionSequence(buf_t *netbuffer);

	void readSnapshotData(std::vector<byte>& buf);
	void writeSnapshotData(std::vector<byte>& buf);

	void readSnapshot(const netdemo_index_entry_t *snap);
	void writeChunk(const byte *data, size_t size, netdemo_message_t type);
	bool writeHeader();
	bool readHeader();

	bool atSnapshotInterval();

	void populateMessageIndexes();

	[[nodiscard]] int getCurrentSnapshotIndex() const;
	[[nodiscard]] int getCurrentMapIndex() const;

	void writeLocalCmd(buf_t *netbuffer) const;
	bool readMessageHeader(netdemo_message_t &type, uint32_t &len, uint32_t &tic) const;
	void readMessageBody(buf_t *netbuffer, uint32_t len);

	std::vector<netdemo_index_entry_t> snapshot_index;
	std::vector<netdemo_index_entry_t> map_index;

	int					pause_netdemotic;
	int					last_map_tic;
};
