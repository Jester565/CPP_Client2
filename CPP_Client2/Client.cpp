#include "Client.h"
#include "TCPConnection.h"
#include "HeaderManager.h"
#include "PacketManager.h"
#include "UDPConnection.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <google/protobuf/any.h>
#include <iostream>
using namespace boost::asio::ip;

Client::Client()
	:ioService(nullptr), tcpConnection(nullptr), connectionState(boost::logic::indeterminate), ipV6(false)
{
}

void Client::createManagers()
{
	packetManager = new PacketManager(this);
}

void Client::initClient(const std::string& ip, const std::string& port, bool v6)
{
	this->ipV6 = v6;
	connectionState = boost::logic::indeterminate;
	ioService = new boost::asio::io_service();
	tcp::resolver resolver(*ioService);
	tcp::resolver::query connectionPoints(ip, port);
	resolver.async_resolve(connectionPoints, boost::bind(&Client::resolveHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::iterator));
	boost::shared_ptr<boost::thread> serviceThread(new boost::thread(boost::bind(&boost::asio::io_service::run, ioService)));
	serviceThread->detach();
}

HeaderManager * Client::createHeaderManager()
{
	return new HeaderManager(this);
}

Client::~Client()
{
	if (packetManager != nullptr)
	{
		delete packetManager;
		packetManager = nullptr;
	}
	if (ioService != nullptr)
	{
		ioService->stop();
	}
	google::protobuf::ShutdownProtobufLibrary();
}

void Client::resolveHandler(const boost::system::error_code & ec, boost::asio::ip::tcp::resolver::iterator epIter)
{
	if (ec)
	{
		std::cerr << "Resolve Error: " << ec.message() << std::endl;
	}
	boost::asio::ip::tcp::socket* tcpSocket = new tcp::socket(*ioService);
	tcp::resolver::iterator epIterEnd;
	boost::system::error_code connectEC;
	while (epIter != epIterEnd)
	{
		connectEC.clear();
		tcpSocket->close();
		tcpSocket->connect(*epIter, connectEC);
		if (!connectEC)
		{
			onConnect(tcpSocket);
			return;
		}
		if (connectEC.value() != boost::asio::error::host_not_found)
		{
			std::cerr << "Resolve EP Error: " << connectEC.message() << std::endl;
		}
		epIter++;
	}
	onReject(tcpSocket);
}

void Client::udpConnect(boost::asio::ip::tcp::endpoint* tcpLocalEP, boost::asio::ip::tcp::endpoint * tcpRemoteEP)
{
	udp::endpoint localEP(tcpLocalEP->address(), tcpLocalEP->port());
	udp::socket* socket;
	if (ipV6)
	{
		socket = new udp::socket(*ioService, udp::v6());
	}
	else
	{
		socket = new udp::socket(*ioService, udp::v4());
	}
	socket->bind(localEP);
	udp::endpoint remoteEP(tcpRemoteEP->address(), tcpRemoteEP->port());
	socket->connect(remoteEP);
	udpConnection = boost::make_shared<UDPConnection>(this, socket);
	udpConnection->read();
}

void Client::ioServiceHandler()
{
	ioService->run();
	std::cout << "IOSERVICE STOPPED" << std::endl;
	delete ioService;
	ioService = nullptr;
}

void Client::onConnect(boost::asio::ip::tcp::socket* tcpSocket)
{
	std::cout << "Successfully connected to - " << tcpSocket->remote_endpoint().address() << ":" << tcpSocket->remote_endpoint().port() << std::endl;
	connectionState = true;
	udpConnect(&tcpSocket->local_endpoint(), &tcpSocket->remote_endpoint());
	tcpConnection = boost::make_shared<TCPConnection>(this, tcpSocket);
	tcpConnection->start();
}

void Client::onReject(boost::asio::ip::tcp::socket* tcpSocket)
{
	std::cout << "Connection refused" << std::endl;
	if (ioService != nullptr)
	{
		ioService->stop();
	}
	connectionState = false;
}
