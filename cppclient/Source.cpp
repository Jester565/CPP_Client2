#include "Client.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <iostream>

int main()
{
	system("pause");
	Client* client = new Client();
	client->createManagers();
	client->initClient("localhost", "5652", true);
	system("pause");
	delete client;
	client = nullptr;
	system("pause");
}