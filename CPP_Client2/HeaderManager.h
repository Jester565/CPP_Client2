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

	virtual boost::shared_ptr<std::vector<unsigned char>> encryptHeader(boost::shared_ptr<OPacket> pack);

	virtual boost::shared_ptr<IPacket> decryptHeader(std::vector<unsigned char>* data, unsigned int size);

	virtual ~HeaderManager();

protected:
	virtual boost::shared_ptr<std::vector<unsigned char>> encryptHeaderAsBigEndian(boost::shared_ptr<OPacket> pack);

	virtual boost::shared_ptr<std::vector<unsigned char>> encryptHeaderToBigEndian(boost::shared_ptr<OPacket> pack);

	virtual boost::shared_ptr<IPacket> decryptHeaderAsBigEndian(std::vector<unsigned char>* data, unsigned int size);

	virtual boost::shared_ptr<IPacket> decryptHeaderFromBigEndian(std::vector<unsigned char>* data, unsigned int size);

	Client* client;

	bool bEndian;
};
