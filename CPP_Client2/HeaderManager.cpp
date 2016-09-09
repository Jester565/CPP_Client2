#include "HeaderManager.h"
#include "Packets/PackFW.pb.h"
#include "OPacket.h"
#include "IPacket.h"
#include <boost/make_shared.hpp>

HeaderManager::HeaderManager(Client* client)
	:client(client)
{
	union {
		uint32_t i;
		char c[4];
	} tbend = { 0x01020304 };
	bEndian = (tbend.c[0] == 1);
}

boost::shared_ptr<std::vector<unsigned char>> HeaderManager::encryptHeader(boost::shared_ptr<OPacket> oPack)
{
	if (bEndian)
	{
		return encryptHeaderAsBigEndian(oPack);
	}
	return encryptHeaderToBigEndian(oPack);
}

boost::shared_ptr<IPacket> HeaderManager::decryptHeader(std::vector<unsigned char>* data, unsigned int size)
{
	if (bEndian)
	{
		return decryptHeaderAsBigEndian(data, size);
	}
	return decryptHeaderFromBigEndian(data, size);
}


HeaderManager::~HeaderManager()
{
}

boost::shared_ptr<std::vector<unsigned char>> HeaderManager::encryptHeaderAsBigEndian(boost::shared_ptr<OPacket> oPack)
{
	if (oPack->getData() == nullptr)
	{
		throw std::invalid_argument("No Data in the OPacket");
	}
	ProtobufPackets::PackHeaderIn phOut;	//is called PackHeaderIn because "In" is what this packet is relative to the server, confusing
	phOut.set_lockey(oPack->locKey);
	phOut.set_serverread(oPack->serverRead);
	for (int i = 0; i < oPack->sendToIDs.size(); i++)
	{
		phOut.add_sendtoids(oPack->sendToIDs.at(i));
	}
	std::string headerPackStr = phOut.SerializeAsString();
	boost::shared_ptr<std::vector<unsigned char>> dataOut = boost::make_shared<std::vector<unsigned char>>(HSI_OUT_SIZE + headerPackStr.size() + oPack->getData()->size());
	dataOut->at(0) = headerPackStr.size() & 0xff;
	dataOut->at(1) = (headerPackStr.size() >> 8) & 0xff;
	std::copy(headerPackStr.begin(), headerPackStr.end(), dataOut->begin() + HSI_IN_SIZE);
	std::copy(oPack->getData()->begin(), oPack->getData()->end(), dataOut->begin() + HSI_IN_SIZE + headerPackStr.size());
	return dataOut;
}

boost::shared_ptr<std::vector<unsigned char>> HeaderManager::encryptHeaderToBigEndian(boost::shared_ptr<OPacket> oPack)
{
	if (oPack->getData() == nullptr)
	{
		throw std::invalid_argument("No Data in the OPacket");
	}
	ProtobufPackets::PackHeaderIn phOut;
	phOut.set_lockey(oPack->locKey);
	phOut.set_serverread(oPack->serverRead);
	for (int i = 0; i < oPack->sendToIDs.size(); i++)
	{
		phOut.add_sendtoids(oPack->sendToIDs.at(i));
	}
	std::string headerPackStr = phOut.SerializeAsString();
	boost::shared_ptr<std::vector<unsigned char>> dataOut = boost::make_shared<std::vector<unsigned char>>(HSI_OUT_SIZE + headerPackStr.size() + oPack->getData()->size());
	dataOut->at(1) = headerPackStr.size() & 0xff;
	dataOut->at(0) = (headerPackStr.size() >> 8) & 0xff;
	std::copy(headerPackStr.begin(), headerPackStr.end(), dataOut->begin() + HSI_IN_SIZE);
	std::copy(oPack->getData()->begin(), oPack->getData()->end(), dataOut->begin() + HSI_IN_SIZE + headerPackStr.size());
	return dataOut;
}

boost::shared_ptr<IPacket> HeaderManager::decryptHeaderAsBigEndian(std::vector<unsigned char>* data, unsigned int size)
{
	boost::shared_ptr<IPacket> iPack = boost::make_shared<IPacket>();
	unsigned int headerPackSize = ((data->at(1) & 0xff) << 8) | (data->at(0) & 0xff);
	ProtobufPackets::PackHeaderOut phIn;
	phIn.ParseFromArray(data->data() + HSI_IN_SIZE, headerPackSize);
	iPack->locKey[0] = phIn.lockey()[0];
	iPack->locKey[1] = phIn.lockey()[1];
	iPack->locKey[2] = '\0';
	iPack->senderID = phIn.sentfromid();
	unsigned int mainPackDataSize = size - headerPackSize - HSI_IN_SIZE;
	boost::shared_ptr<std::string> mainPackDataStr = boost::make_shared<std::string>(data->begin() + HSI_IN_SIZE + headerPackSize, data->begin() + size);
	iPack->data = mainPackDataStr;
	return iPack;
}

boost::shared_ptr<IPacket> HeaderManager::decryptHeaderFromBigEndian(std::vector<unsigned char>* data, unsigned int size)
{
	boost::shared_ptr<IPacket> iPack = boost::make_shared<IPacket>();
	unsigned int headerPackSize = ((data->at(0) & 0xff) << 8) | (data->at(1) & 0xff);
	ProtobufPackets::PackHeaderOut phIn;
	phIn.ParseFromArray(data->data() + HSI_IN_SIZE, headerPackSize);
	iPack->locKey[0] = phIn.lockey()[0];
	iPack->locKey[1] = phIn.lockey()[1];
	iPack->locKey[2] = '\0';
	iPack->senderID = phIn.sentfromid();
	unsigned int mainPackDataSize = size - headerPackSize - HSI_IN_SIZE;
	boost::shared_ptr<std::string> mainPackDataStr = boost::make_shared<std::string>(data->begin() + HSI_IN_SIZE + headerPackSize, data->begin() + size);
	iPack->data = mainPackDataStr;
	return iPack;
}
