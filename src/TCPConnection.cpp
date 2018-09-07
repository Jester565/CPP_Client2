#include "TCPConnection.h"
#include "Client.h"
#include "IPacket.h"
#include "OPacket.h"
#include "HeaderManager.h"
#include "PacketManager.h"
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <iostream>

int TCPConnection::SentCount = 0;
int TCPConnection::SentBytes = 0;

int TCPConnection::ReceiveCount = 0;

TCPConnection::TCPConnection(Client* client, boost::asio::ip::tcp::socket* boundSocket)
	:socket(boundSocket), client(client), receiveStorage(nullptr), alive(true), hm(nullptr), sending(false)
{
	std::cout << "TCPRemoteEP: " << socket->remote_endpoint().address().to_string() << " : " << socket->remote_endpoint().port() << " : " << socket->remote_endpoint().address().is_v6() << std::endl;
	std::cout << "TCPLocalEP: " << socket->local_endpoint().address().to_string() << " : " << socket->local_endpoint().port() << " : " << socket->local_endpoint().address().is_v6() << std::endl;
	hm = client->createHeaderManager();
}

void TCPConnection::start()
{
	read();
}

void TCPConnection::read()
{
	if (alive)
	{
		if (receiveStorage == nullptr)
		{
			receiveStorage = new std::vector<unsigned char>();
			receiveStorage->resize(MAX_DATA_SIZE);
		}
		socket->async_read_some(boost::asio::buffer(*receiveStorage, MAX_DATA_SIZE), boost::bind(&TCPConnection::asyncReceiveHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
}

void TCPConnection::asyncReceiveHandler(const boost::system::error_code& error, unsigned int nBytes)
{
	ReceiveCount++;
	if (error)
	{
		if (error == boost::asio::error::connection_reset)
		{
			std::cout << "Connection Closed" << std::endl;
			//TODO: disconnect action
			return;
		}
		std::cerr << "Error occured in TCP Reading: " << error << " - " << error.message() << std::endl;
		return;
	}
	boost::shared_ptr<IPacket> iPack = hm->decryptHeader(receiveStorage, nBytes);
	if (iPack != nullptr)
	{
		client->getPacketManager()->process(iPack);
	}
	read();
}


void TCPConnection::send(boost::shared_ptr<OPacket> oPack)
{
	boost::shared_ptr<std::vector <unsigned char>> sendData = hm->encryptHeader(oPack);
	if (sendData->size() > MAX_DATA_SIZE)
	{
		std::cerr << "Data of " << oPack->getLocKey() << " had a size of " << sendData->size() << " exceeding the MAX_DATA_SIZE of " << MAX_DATA_SIZE << std::endl;
	}
	/*
	while (true)
	{
		sendingMutex.lock();
		if (!sending)
		{
			break;
		}
		std::cout << "BLOCKED" << std::endl;
		sendingMutex.unlock();
	}
	sending = true;
	sendingMutex.unlock();
	*/
	SentBytes += sendData->size();
	boost::asio::async_write(*socket, boost::asio::buffer(*sendData, sendData->size()), boost::bind(&TCPConnection::asyncSendHandler, shared_from_this(), boost::asio::placeholders::error, sendData));
	//socket->async_write_some(boost::asio::buffer(*sendData, sendData->size()), boost::bind(&TCPConnection::asyncSendHandler, shared_from_this(), boost::asio::placeholders::error, sendData));
}

void TCPConnection::send(boost::shared_ptr<std::vector<unsigned char>> sendData)
{
	if (sendData->size() > MAX_DATA_SIZE)
	{
		std::cerr << "Raw data had a size of " << sendData->size() << " exceeding the MAX_DATA_SIZE of " << MAX_DATA_SIZE << std::endl;
	}
	while (true)
	{
		sendingMutex.lock();
		if (!sending)
		{
			break;
		}
		sendingMutex.unlock();
	}
	sending = true;
	sendingMutex.unlock();
	socket->async_write_some(boost::asio::buffer(*sendData, sendData->size()), boost::bind(&TCPConnection::asyncSendHandler, shared_from_this(), boost::asio::placeholders::error, sendData));
}

void TCPConnection::asyncSendHandler(const boost::system::error_code& error, boost::shared_ptr<std::vector<unsigned char>> sendData)
{
	/*
	sendingMutex.lock();
	sending = false;
	sendingMutex.unlock();
	*/
	SentCount++;
	if (error)
	{
		if (error == boost::asio::error::connection_reset)
		{
			//TODO: disconnect action
		}
		std::cerr << "An error occured in TCP Sending: " << error.message() << std::endl;
		return;
	}
}

TCPConnection::~TCPConnection()
{
	if (socket != nullptr)
	{
		boost::system::error_code ec;
		socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		if (ec)
		{
			std::cerr << "Error when closing TCPConnection: " << ec.message() << std::endl;
		}
		socket->close();
	}
	delete hm;
	hm = nullptr;
	if (receiveStorage != nullptr)
	{
		delete receiveStorage;
		receiveStorage = nullptr;
	}
	if (socket != nullptr)
	{
		delete socket;
		socket = nullptr;
	}
}
