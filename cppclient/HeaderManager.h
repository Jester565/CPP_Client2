#pragma once
#include "Macros.h"
#include <boost/shared_ptr.hpp>
#include <cstdint>
#include <vector>

class Client;
class OPacket;
class IPacket;

class HeaderManager
{
public:
	const static uint8_t HSI_OUT_SIZE = 2;
	const static uint8_t HSI_IN_SIZE = 2;
	HeaderManager(Client* client);

	virtual unsigned int getInitialReceiveSize() {
		return HSI_IN_SIZE;
	}

	virtual boost::shared_ptr<std::vector<unsigned char>> serializePacket(boost::shared_ptr<OPacket> pack);

	virtual boost::shared_ptr<IPacket> parsePacket(unsigned char* data, unsigned int size, unsigned int& bytesToReceive);

	virtual ~HeaderManager();

protected:
	unsigned int parseStage;

	boost::shared_ptr<IPacket> activeIPacket;

	Client* client;

	bool bEndian;
};
