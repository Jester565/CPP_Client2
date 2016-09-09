#include "OPacket.h"
#include "IPacket.h"

OPacket::OPacket(IPacket * iPack, bool copyData, bool serverRead)
	:serverRead(serverRead)
{
	setLocKey(iPack->getLocKey());
	addSendToID(iPack->getSenderID());
	if (copyData)
	{
		data = iPack->getData();
	}
}

OPacket::OPacket(const char * loc, bool serverRead)
	:serverRead(serverRead)
{
	setLocKey(loc);
}

OPacket::OPacket(const char * loc, IDType sendToID, bool serverRead)
	: serverRead(serverRead)
{
	setLocKey(loc);
	addSendToID(sendToID);
}

OPacket::OPacket(const char * loc, IDType * sendToIDs, uint8_t size, bool serverRead)
	:serverRead(serverRead)
{
	setLocKey(loc);
	std::copy(sendToIDs, sendToIDs + size, sendToIDs);
}

OPacket::~OPacket()
{
}
