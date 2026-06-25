#pragma once

#include "i_net.h"
#include <deque>
#include "netdemo.h"

class ServerNetDemo : public NetDemo
{
public:
    ServerNetDemo();
	~ServerNetDemo();
	ServerNetDemo(const ServerNetDemo &rhs);
	ServerNetDemo& operator=(const ServerNetDemo &rhs);

	bool startRecording(const std::string &filename);
	bool stopRecording();

	void writeMessages(int num_players);
	void capture(const buf_t* netbuffer);
	void capture(const std::basic_string<byte>& buffer);
	void writeMapChange();
	void writeIntermission();
    void writeSnapshotInterval();
    bool atSnapshotInterval();


	void ticker();
	[[nodiscard]] int calculateTimeElapsed() const;
	[[nodiscard]] int calculateTotalTime() const;
	[[nodiscard]] const std::vector<int> getMapChangeTimes() const;
	[[nodiscard]] const std::string &getFileName() const { return filename; }

    std::deque<buf_t>   captured;

private:

	void cleanUp();
	void copy(ServerNetDemo &to, const ServerNetDemo &from);
	void error(const std::string &message);
	void fatalError(const std::string &message);
	void reset();

	void writeLauncherSequence(buf_t *netbuffer);
	void writeConnectionSequence(buf_t *netbuffer);

	void writeSnapshotData(std::vector<byte>& buf);

	void writeChunk(const byte *data, size_t size, netdemo_message_t type);
	bool writeHeader();

	void writeLocalCmd(buf_t *netbuffer) const;


	int					last_map_tic;
};
