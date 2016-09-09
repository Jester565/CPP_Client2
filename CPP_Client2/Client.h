#pragma once
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/thread.hpp>
#include <thread>
#include <mutex>
#include <stdint.h>
#include <string>

class TCPConnection;
class UDPConnection;
class PacketManager;
class HeaderManager;

class Client
{
public:
	Client();
	void createManagers();
	void initClient(const std::string& ip, const std::string& port, bool v6);
	HeaderManager* createHeaderManager();
	PacketManager* getPacketManager()
	{
		return packetManager;
	}
	boost::shared_ptr<TCPConnection> getTCPConnection()
	{
		return tcpConnection;
	}
	boost::logic::tribool getConnectionState()
	{
		return connectionState;
	}

	~Client();

protected:
	void resolveHandler(const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator epIter);
	void udpConnect(boost::asio::ip::tcp::endpoint* localPoint, boost::asio::ip::tcp::endpoint* tcpRemoteEP);
	boost::asio::io_service* ioService;
	boost::shared_ptr<TCPConnection> tcpConnection;
	boost::shared_ptr<UDPConnection> udpConnection;
	PacketManager* packetManager;
	void ioServiceHandler();
	virtual void onConnect(boost::asio::ip::tcp::socket* tcpSocket);
	virtual void onReject(boost::asio::ip::tcp::socket* tcpSocket);
	boost::logic::tribool connectionState;
	bool ipV6;
};
