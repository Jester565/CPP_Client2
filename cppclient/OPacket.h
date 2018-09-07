#pragma once
#include "Macros.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>

class IPacket;

class OPacket
{
public:
	friend class HeaderManager;
	OPacket(IPacket* iPack, bool copyData, bool serverRead = false);
	OPacket(const char* loc = nullptr, bool serverRead = false);
	OPacket(const char* loc, IDType sendToID, bool serverRead = false);
	OPacket(const char* loc, IDType* sendToIDs, uint8_t size, bool serverRead = false);

	void setLocKey(const char* loc)
	{
		if (loc != nullptr)
		{
			locKey[0] = loc[0];
			locKey[1] = loc[1];
		}
		else
		{
			locKey[0] = loc[UNDEFINED_LOC];
			locKey[1] = loc[UNDEFINED_LOC];
		}
		locKey[2] = '\0';
	}

	const char* getLocKey() const
	{
		return locKey;
	}

	void addSendToID(IDType id)
	{
		sendToIDs.push_back(id);
	}

	void setServerRead(bool mode)
	{
		serverRead = mode;
	}

	std::vector <IDType>* getSendToIDs()
	{
		return &sendToIDs;
	}

	void setData(boost::shared_ptr<std::string> data)
	{
		this->data = data;
	}

	boost::shared_ptr<std::string> getData()
	{
		return data;
	}

	friend std::ostream& operator << (std::ostream& oStream, OPacket& oPack)
	{
		oStream << "OPack -- LocKey: " << oPack.locKey << " SendToIDs: ";
		if (oPack.sendToIDs.size() == 0)
		{
			oStream << "NONE";
		}
		for (int i = 0; i < oPack.sendToIDs.size(); i++)
		{
			oStream << oPack.sendToIDs.at(i);
			if (i < oPack.sendToIDs.size() - 1)
			{
				oStream << ", ";
			}
		}
		oStream << " ServerRead: " << oPack.serverRead << " Data: " << *(oPack.data) << std::endl;
	}

	~OPacket();

private:
	boost::shared_ptr<std::string> data;
	std::vector <IDType> sendToIDs;
	char locKey[3];
	bool serverRead;
};
