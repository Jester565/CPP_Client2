#pragma once
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/thread/mutex.hpp>
#include <vector>
#include <queue>

class OPacket;
class HeaderManager;
class Client;

class TCPConnection : public boost::enable_shared_from_this<TCPConnection>
{
public:
	static int SentCount;
	static int ReceiveCount;
	static int SentBytes;
	static const uint16_t MAX_DATA_SIZE = 40000;
	TCPConnection(Client* client, boost::asio::ip::tcp::socket* boundSocket);

	virtual void start();

	virtual void read(unsigned int receiveSize = 0);

	void send(boost::shared_ptr<OPacket> oPack);

	void send(boost::shared_ptr<std::vector<unsigned char>> sendData);

	boost::asio::ip::tcp::socket* getSocket()
	{
		return socket;
	}

	void kill()
	{
		this->alive = false;
	}

	virtual void asyncReceiveHandler(const boost::system::error_code& error, unsigned int bytes);

	virtual void asyncSendHandler(const boost::system::error_code& error, boost::shared_ptr<std::vector<unsigned char>> sendData);

	virtual ~TCPConnection();

protected:
	std::vector<unsigned char>* receiveStorage;
	boost::asio::ip::tcp::socket* socket;
	boost::mutex sendQueueMutex;
	std::queue <boost::shared_ptr<std::vector <unsigned char>>> sendQueue;
	HeaderManager* hm;
	Client* client;
	int errorMode;
	bool alive;
};

