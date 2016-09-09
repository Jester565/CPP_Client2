#include "UDPConnection.h"
#include "Client.h"
#include "PacketManager.h"
#include "HeaderManager.h"
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <iostream>

using namespace boost::asio::ip;
UDPConnection::UDPConnection(Client* client, boost::asio::ip::udp::socket* socket)
	:client(client), socket(socket), receiveData(nullptr)
{
	std::cout << "UDPRemoteEP: " << socket->remote_endpoint().address().to_string() << " : " << socket->remote_endpoint().port() << " : " << socket->remote_endpoint().address().is_v6() << std::endl;
	std::cout << "UDPRemoteEP: " << socket->local_endpoint().address().to_string() << " : " << socket->local_endpoint().port() << " : " << socket->local_endpoint().address().is_v6() << std::endl;
	hm = client->createHeaderManager();
}

void UDPConnection::read()
{
	if (receiveData == nullptr)
	{
		receiveData = new std::vector<unsigned char>(MAX_RECEIVE_SIZE);
	}
	socket->async_receive(boost::asio::buffer(*receiveData, MAX_RECEIVE_SIZE), boost::bind(&UDPConnection::asyncReceive, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void UDPConnection::send(boost::shared_ptr<OPacket> oPack)
{
	boost::shared_ptr<std::vector<unsigned char>> sendData = hm->encryptHeader(oPack);
	socket->async_send(boost::asio::buffer(*sendData, sendData->size()), boost::bind(&UDPConnection::asyncSend, shared_from_this(), boost::asio::placeholders::error, sendData));
}

UDPConnection::~UDPConnection()
{
	if (socket != nullptr)
	{
		boost::system::error_code ec;
		socket->shutdown(boost::asio::ip::udp::socket::shutdown_both, ec);
		if (ec)
		{
			std::cerr << "Error when closing UDP Socket: " << ec.message() << std::endl;
		}
		socket->close();
	}
	if (receiveData != nullptr)
	{
		delete receiveData;
		receiveData = nullptr;
	}
	delete hm;
	hm = nullptr;
	if (socket != nullptr)
	{
		delete socket;
		socket = nullptr;
	}
}

void UDPConnection::asyncSend(const boost::system::error_code& error, boost::shared_ptr<std::vector<unsigned char>> sendData)
{
	if (error)
	{
		std::cerr << "Error occured in UDP sending: " << error.message() << std::endl;
	}
}

void UDPConnection::asyncReceive(const boost::system::error_code& error, unsigned int nBytes)
{
	if (error)
	{
		if (error == boost::asio::error::connection_reset)
		{
			std::cout << "Connection Closed" << std::endl;
			//TODO: disconnect
			return;
		}
		std::cerr << "Error occured in UDP Reading: " << error << " - " << error.message() << std::endl;
	}
	boost::shared_ptr<IPacket> iPack = hm->decryptHeader(receiveData, nBytes);
	if (iPack != nullptr)
	{
		client->getPacketManager()->process(iPack);
	}
	read();
}
