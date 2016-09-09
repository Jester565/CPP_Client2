#pragma once
#include <boost/asio/ip/udp.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <boost/enable_shared_from_this.hpp>

class HeaderManager;
class Client;
class OPacket;

class UDPConnection : public boost::enable_shared_from_this<UDPConnection>
{
public:
	const static unsigned int MAX_RECEIVE_SIZE = 1024;
	UDPConnection(Client* client, boost::asio::ip::udp::socket* socket);

	void read();

	void send(boost::shared_ptr<OPacket> oPack);

	~UDPConnection();

protected:
	void asyncSend(const boost::system::error_code& ec, boost::shared_ptr<std::vector<unsigned char>> sendData);
	void asyncReceive(const boost::system::error_code& ec, unsigned int nBytes);

	HeaderManager* hm;
	Client* client;

	boost::asio::ip::udp::socket* socket;
	std::vector<unsigned char>* receiveData;
};

