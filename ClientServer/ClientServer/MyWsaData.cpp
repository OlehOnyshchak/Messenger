#include "ClientServer.h"
#include <iostream>

ClientServer::MyWsaData::MyWsaData()
{
	if (WSAStartup(MY_WINSOCK_VERSION, &wsa_data_))
	{
		std::cerr << "WSAdata initialization failed with an error: "
			<< (int)WSAGetLastError << std::endl;

		uploaded_ = false;
	}
	else
	{
		uploaded_ = true;
	}
}

ClientServer::MyWsaData::~MyWsaData()
{
	if (WSACleanup())
		std::cerr << "WSACleanup failed with an error: "
		<< (int)WSAGetLastError << std::endl;
}
