#pragma once
#include "PKeyOwner.h"
#include <boost/shared_ptr.hpp>

class Client;

class Tester : public PKeyOwner
{
public:
	Tester(Client* client);

	void keyA0(boost::shared_ptr<IPacket> iPacket);
	void sendTest(const std::string& msg);

	~Tester();

private:
	Client* client;
};
