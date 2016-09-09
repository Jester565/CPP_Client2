#include "Tester.h"
#include "Client.h"
#include "IPacket.h"
#include "OPacket.h"
#include "Packets/Test.pb.h"
#include "TCPConnection.h"
#include <boost/make_shared.hpp>
#include <string>
#include <iostream>

Tester::Tester(Client* client)
	:PKeyOwner(client->getPacketManager()), client(client)
{
	addKey(new PKey("A0", this, &Tester::keyA0));
}

void Tester::keyA0(boost::shared_ptr<IPacket> iPacket)
{
	ProtobufPackets::PackA0 packA0;
	packA0.ParseFromString(*iPacket->getData());
	std::cout << "RECEIVED: " << packA0.msg() << std::endl;
}

void Tester::sendTest(const std::string& msg)
{
	ProtobufPackets::PackA0 packA0;
	packA0.set_msg(msg);
	boost::shared_ptr<OPacket> oPackA0 = boost::make_shared<OPacket>("A0");
	oPackA0->setServerRead(true);
	oPackA0->setData(boost::make_shared<std::string>(packA0.SerializeAsString()));
	client->getTCPConnection()->send(oPackA0);
}

Tester::~Tester()
{
}
