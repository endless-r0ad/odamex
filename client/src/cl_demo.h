#pragma once

#include <deque>
#include <fstream>

#include "netdemo.h"
#include "i_net.h"

class ClientNetDemo : public NetDemo
{
public:
	ClientNetDemo() = default;
	~ClientNetDemo();
	ClientNetDemo(const ClientNetDemo &rhs)             = delete;
	ClientNetDemo& operator=(const ClientNetDemo &rhs)  = delete;

	ClientNetDemo(ClientNetDemo&&) = default;
	ClientNetDemo& operator=(ClientNetDemo&&) = default;


	bool startPlaying(const std::string &filename);
	bool startRecording(const std::string &filename);
	bool stopPlaying();
	bool stopRecording();
	bool pause();
	bool resume();

	void writeMessages();
	void readMessages(buf_t* netbuffer);
	void writeMapChange();
	void writeIntermission();

	void nextTic();
	void prevTic();
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
	void cleanUp();
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

	bool atSnapshotInterval();

	void populateMessageIndexes();

	[[nodiscard]] int getCurrentSnapshotIndex() const;
	[[nodiscard]] int getCurrentMapIndex() const;

	void writeLocalCmd(buf_t *netbuffer) const;
	bool readMessageHeader(netdemo_message_t &type, uint32_t &len, uint32_t &tic);
	void readMessageBody(buf_t *netbuffer, uint32_t len);

	std::vector<netdemo_index_entry_t> snapshot_index{};
	std::vector<netdemo_index_entry_t> map_index     {};

	int                 pause_netdemotic{ 0 };
	int                 last_map_tic    { 0 };
};
