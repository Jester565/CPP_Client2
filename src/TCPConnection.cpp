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
	:socket(boundSocket), client(client), receiveStorage(nullptr), alive(true), hm(nullptr)
{
	std::cout << "TCPRemoteEP: " << socket->remote_endpoint().address().to_string() << " : " << socket->remote_endpoint().port() << " : " << socket->remote_endpoint().address().is_v6() << std::endl;
	std::cout << "TCPLocalEP: " << socket->local_endpoint().address().to_string() << " : " << socket->local_endpoint().port() << " : " << socket->local_endpoint().address().is_v6() << std::endl;
	hm = client->createHeaderManager();
}

void TCPConnection::start()
{
	receiveStorage = new std::vector<unsigned char>();
	receiveStorage->resize(MAX_DATA_SIZE);
	read(hm->getInitialReceiveSize());
}

void TCPConnection::read(unsigned int receiveSize)
{
	if (alive)
	{
		if (receiveSize > 0) {
			//Link the asyncReceiveCallback to be called when data size is received
			boost::asio::async_read(*socket, boost::asio::buffer(*receiveStorage, MAX_DATA_SIZE), boost::asio::transfer_exactly(receiveSize), boost::bind(&TCPConnection::asyncReceiveHandler, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
		else {
			//Receive some unspecified amount of data
			socket->async_read_some(boost::asio::buffer(*receiveStorage, MAX_DATA_SIZE), boost::bind(&TCPConnection::asyncReceiveHandler, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
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

	unsigned int bytesToRead = 0;
	boost::shared_ptr<IPacket> iPack = hm->parsePacket(receiveStorage->data(), nBytes, bytesToRead);
	if (iPack != nullptr) {
		client->getPacketManager()->process(iPack);
	}
	//begin reading again
	read(bytesToRead);
}


void TCPConnection::send(boost::shared_ptr<OPacket> oPack)
{
	boost::shared_ptr<std::vector <unsigned char>> sendData = hm->serializePacket(oPack);
	send(sendData);
}

//send raw data to the client
void TCPConnection::send(boost::shared_ptr<std::vector<unsigned char>> sendData)
{
	sendQueueMutex.lock();
	bool empty = sendQueue.empty();
	sendQueue.push(sendData);
	if (empty)
	{
		boost::asio::async_write(*socket, boost::asio::buffer(*sendData, sendData->size()), boost::bind(&TCPConnection::asyncSendHandler, shared_from_this(), boost::asio::placeholders::error, sendData));
	}
	sendQueueMutex.unlock();
}

void TCPConnection::asyncSendHandler(const boost::system::error_code& error, boost::shared_ptr<std::vector<unsigned char>> sendData)
{
	if (error)
	{
		if (error == boost::asio::error::connection_reset)
		{
			//TODO: disconnect action
		}
		std::cerr << "An error occured in TCP Sending: " << error.message() << std::endl;
		return;
	}
	
	sendQueueMutex.lock();
	sendQueue.pop();
	if (!sendQueue.empty()) {
		boost::shared_ptr<std::vector<unsigned char>> sendData = sendQueue.front();
		boost::asio::async_write(*socket, boost::asio::buffer(*sendData, sendData->size()), boost::bind(&TCPConnection::asyncSendHandler, shared_from_this(), boost::asio::placeholders::error, sendData));
	}
	sendQueueMutex.unlock();
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
