#include "Client.h"
#include "Tester.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <iostream>

int main()
{
	system("pause");
	Client* client = new Client();
	client->createManagers();
	Tester* tester = new Tester(client);
	client->initClient("localhost", "5652", true);
	system("pause");
	//tester->sendTest("RORK");
	//system("pause");
	delete tester;
	tester = nullptr;
	delete client;
	client = nullptr;
	system("pause");
}