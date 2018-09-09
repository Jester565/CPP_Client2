#include "HeaderManager.h"
#include "PackFW.pb.h"
#include "OPacket.h"
#include "IPacket.h"
#include <boost/make_shared.hpp>

HeaderManager::HeaderManager(Client* client)
	:client(client), parseStage(0), activeIPacket(nullptr)
{
	union {
		uint32_t i;
		char c[4];
	} tbend = { 0x01020304 };
	bEndian = (tbend.c[0] == 1);
}

boost::shared_ptr<std::vector<unsigned char>> HeaderManager::serializePacket(boost::shared_ptr<OPacket> oPack)
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
	phOut.set_datasize(oPack->getData()->size());
	std::string headerPackStr = phOut.SerializeAsString();
	boost::shared_ptr<std::vector<unsigned char>> dataOut = boost::make_shared<std::vector<unsigned char>>(HSI_OUT_SIZE + headerPackStr.size() + oPack->getData()->size());
	if (bEndian) {
		dataOut->at(0) = headerPackStr.size() & 0xff;
		dataOut->at(1) = (headerPackStr.size() >> 8) & 0xff;
	}
	else {
		dataOut->at(1) = headerPackStr.size() & 0xff;
		dataOut->at(0) = (headerPackStr.size() >> 8) & 0xff;
	}
	std::copy(headerPackStr.begin(), headerPackStr.end(), dataOut->begin() + HSI_IN_SIZE);
	std::copy(oPack->getData()->begin(), oPack->getData()->end(), dataOut->begin() + HSI_IN_SIZE + headerPackStr.size());
	return dataOut;
}

boost::shared_ptr<IPacket> HeaderManager::parsePacket(unsigned char * data, unsigned int size, unsigned int& bytesToReceive)
{
	if (parseStage == 0) {
		parseStage++;
		activeIPacket = boost::make_shared<IPacket>();
		if (bEndian) {
			bytesToReceive = ((data[1] & 0xff) << 8) | (data[0] & 0xff);
		}
		else {
			bytesToReceive = ((data[0] & 0xff) << 8) | (data[1] & 0xff);
		}
		return nullptr;
	}
	if (parseStage == 1) {
		ProtobufPackets::PackHeaderOut phIn;
		phIn.ParseFromArray(data, size);
		activeIPacket->locKey[0] = phIn.lockey()[0];
		activeIPacket->locKey[1] = phIn.lockey()[1];
		activeIPacket->locKey[2] = '\0';
		activeIPacket->senderID = phIn.sentfromid();
		if (phIn.datasize() == 0) {
			parseStage = 0;
			activeIPacket->data = boost::make_shared<std::string>();
			bytesToReceive = getInitialReceiveSize();
			auto iPack = activeIPacket;
			activeIPacket = nullptr;
			return iPack;
		}
		else {
			parseStage++;
			bytesToReceive = phIn.datasize();
			return nullptr;
		}
	}
	if (parseStage == 2) {
		parseStage = 0;
		activeIPacket->data = boost::make_shared<std::string>((char*)data, size);
		auto iPack = activeIPacket;
		activeIPacket = nullptr;
		bytesToReceive = getInitialReceiveSize();
		return iPack;
	}
	return nullptr;
}

HeaderManager::~HeaderManager()
{
}
