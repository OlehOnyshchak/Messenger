#ifdef _WIN32
#include "Server.h"
#include <iostream>

using std::cerr;
using std::endl;


bool Server::Work(char* port, char* host)
{
	// creating, binding and listeing socket
	if (!wsa_data_.get_uploaded_() || !ConfigurateSocket(port, host))
		return false;

	const bool kRunning = true;
	while (kRunning)
	{
		Connection new_connect;

		// waiting for incoming connections
		if (!AcceptRequest(new_connect))
			return false;

//		DWORD nThreadID;
//		CreateThread(0, 0, NewRequestHandler, (void*)sd, 0, &nThreadID);
		// there new thread will be created
		NewRequestHandler(new_connect);
	}

}

void Server::NewRequestHandler(Connection& connection)
{
	// message-confirmation which say that server is avaiable for processing 
	// client requests
	char* mess = "You are welcome in Paradise server. Please, write down your request\n";
	connection.CreateMessage(MESSAGE_TYPE, strlen(mess));
	connection.set_repository(mess, MESSAGE_TYPE);

	if (!SendWholeMess(connection))
		return;

	mess = "Sorry, but this information is currently unavailable. Please, check it later\n";
	int result = 0;
	do {
		//reciving request
		result = RecvMess(connection);

		// giving the reply when request is recived 
		switch (result)
		{
		case CONNECTION_GRACEFFULLY_CLOSED:
			connection.set_repository("Have a good day!\n", MESSAGE_TYPE);
			SendWholeMess(connection);
			break;

		case MESSAGE_WAS_TRANSMITTED:
			connection.set_repository(mess, MESSAGE_TYPE);
			SendWholeMess(connection);
			break;

		case FILE_WAS_TRANSMITTED:
			connection.set_repository("File was transmitted!\n", MESSAGE_TYPE);
			SendWholeMess(connection);
			break;
			
		default:
			// some kind of error occured, will be logged in futher versions
			break;
		}
	} while (result > 0);

	ShutdownSocket(connection);
}

bool Server::SendWholeMess(Connection& connection_)
{
	connection_.CreateHeader();
	SendHeader(connection_);

	int sended_bytes = 0;
	while (sended_bytes != connection_.get_packet_lenght())
	{
		int chunk_size = connection_.TakeChunk();
		if (!SendMess(connection_, chunk_size))
			return false;

		sended_bytes += chunk_size;
	}
	return true;
}


bool Server::ConfigurateSocket(char * port, char * ip)
{
	return CreateAddrinfo(port) &&
		CreateSocket() &&
		BindSocket() &&
		ListenRequest();
}

bool Server::CreateAddrinfo(char* port)
{
	bool returned_val = true;

	addrinfo hint;

	ZeroMemory(&hint, sizeof(hint));
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = IPPROTO_TCP;
	hint.ai_flags = AI_PASSIVE;


	if (getaddrinfo(NULL, port, &hint, &res_))
	{
		cerr << "getadrrinfo failed with an error: "
			<< (int)WSAGetLastError << endl;

		returned_val = false;
	}

	return returned_val;
}

bool Server::CreateSocket()
{
	bool returned_val = true;

	listening_ = socket(res_->ai_family, res_->ai_socktype, res_->ai_protocol);
	if (listening_ == INVALID_SOCKET)
	{
		cerr << "Creation of a socket failed with an error: "
			<< (int)WSAGetLastError << endl;

		returned_val = false;
	}

	return returned_val;
}


bool Server::BindSocket()
{
	bool returned_val = true;
	int result = bind(listening_, res_->ai_addr, res_->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		cerr << "Binding a socket failed with an error: "
			<< (int)WSAGetLastError << endl;

		returned_val = false;
	}

	return returned_val;
}

bool Server::ListenRequest()
{
	bool returned_val = true;
	// in this version server are able to communicate only with
	// one client at the same time
	int result = listen(listening_, 1);
	if (result == SOCKET_ERROR)
	{
		cerr << "Listening for incoming connections failed with an error: "
			<< (int)WSAGetLastError << endl;

		returned_val = false;
	}
	return returned_val;
}



bool Server::AcceptRequest(Connection& connect)
{
	// initializing structure to hold client's IP
	sockaddr_storage client_ip;
	int cliet_ip_size = sizeof(client_ip);

	connect.set_sock_(accept(listening_, (sockaddr*)&client_ip, &cliet_ip_size));
	if (connect.get_sock_() != INVALID_SOCKET)
	{
		connect.record_id(client_ip);
		std::cout << "Connection established with " << connect.get_clients_ip() << endl;
		connect.connected();
		return true;
	}
	else
	{
		cerr << "accept failed with an error: "	<< WSAGetLastError() << endl;
		return false;
	}
}

// indicate that Server has no more information to pass
bool Server::ShutdownSocket(Connection& connect)
{
	int result = shutdown(connect.get_sock_(), SD_SEND);
	if (result == SOCKET_ERROR) 
	{
		cerr << "shutdown failed with an error: "
			<< WSAGetLastError() << endl;

		return false;
	}

	std::cout << "Connection closed" << endl << endl;
	closesocket(connect.get_sock_());
	connect.disconnected();

	return true;
}


Server::~Server()
{
	freeaddrinfo(res_);
	closesocket(listening_);
}
#endif