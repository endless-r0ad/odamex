#pragma once

#include <deque>
#include <fstream>

#include "netdemo.h"
#include "i_net.h"

class ServerNetDemo : public NetDemo
{
public:
    ServerNetDemo() = default;
	~ServerNetDemo();
	ServerNetDemo(const ServerNetDemo &rhs)             = delete;
	ServerNetDemo& operator=(const ServerNetDemo &rhs)  = delete;

	ServerNetDemo(ServerNetDemo&&) = default;
	ServerNetDemo& operator=(ServerNetDemo&&) = default;


	bool startRecording(const std::string &filename);
	bool stopRecording();

	void writeMessages(bool isNetdemoStartup = false);
	void writeMapChange();
	void writeIntermission();
	bool atSnapshotInterval();

	void ticker();
	[[nodiscard]] int calculateTimeElapsed() const;
	[[nodiscard]] int calculateTotalTime() const;
	[[nodiscard]] const std::vector<int> getMapChangeTimes() const;
	[[nodiscard]] const std::string &getFileName() const { return filename; }

private:
	void cleanUp();
	void error(const std::string &message);
	void reset();

	void writeLauncherSequence(buf_t *netbuffer);
	void writeConnectionSequence(buf_t *netbuffer);

	void writeSnapshotData(std::vector<byte>& buf);

	void writeChunk(const byte *data, size_t size, netdemo_message_t type);

	void writeLocalCmd(buf_t *netbuffer) const;

	int                 last_map_tic    { 0 };
};
