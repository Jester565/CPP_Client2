#pragma once
#include "Macros.h"
#include <boost/shared_ptr.hpp>
#include <string>

class IPacket
{
public:
	friend class HeaderManager;

	IPacket();

	IDType getSenderID() const
	{
		return senderID;
	}

	const char* getLocKey() const
	{
		return locKey;
	}

	const boost::shared_ptr <std::string> getData() const
	{
		return data;
	}

	friend std::ostream& operator << (std::ostream& oStream, IPacket& iPack)
	{
		oStream << "IPack -- LocKey: " << iPack.locKey << " SenderID" << iPack.senderID << " Data" << *iPack.data << std::endl;
	}

	~IPacket();

private:
	char locKey[3];
	IDType senderID;
	boost::shared_ptr<std::string> data;
};

