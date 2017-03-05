#include "Client.h"
#include <iostream>
#include <memory>
using std::cerr;
using std::endl;

using std::cin;


bool Client::Work(char* host, char* ip)
{
	// initialized socket and connected it to predefined server
	if (!wsa_data_.get_uploaded_() ||
		!ConfigurateSocket(host))
		return false;

	// blocking recv method which would be done
	// when server send to client welcome message.
	// That is, server starts to servise the client
	if (RecvMess() != 1)
		return false;

	const bool kRunning = true;
	while (kRunning)
	{
		// sending user's request
		int result = SendWholeMess();
		if (result == 0)
		{
			// stop sending
			break;
		}
		else if (result < 0)
		{
			// error occured
			std::cout << "Connection problems with servers are occured!" << endl; 
			return false;
		}

		// waiting for the responce
		
		if (RecvMess() != 1)
			return false;
	}

	// notifying the server, that we would not sending request anymore 
	if (!ShutdownConnection())
		return false;

	// checking whether we did not miss any information from the server
	while (RecvMess() != CONNECTION_GRACEFFULLY_CLOSED)
	{
		// repeats while server closed connection
	}

	// closing connection
	return Clean();
}


bool Client::ConfigurateSocket(char* port, char* host)
{
	return CreateAddrinfo(host) && EstablishConnection();
}

int Client::SendMess(int sending_bytes)
{
	return ClientServer::SendMess(connection_, sending_bytes);
}

int Client::SendHeader()
{
	return ClientServer::SendHeader(connection_);
}

int Client::RecvMess()
{
	return ClientServer::RecvMess(connection_);
}

// returns:
// 1 - if sending was successful
// 0 - client closed program
// SOCKET_ERROR -  if sending failed.
// ESTABLISHING_CONNECTION_ERROR - if socket is not connected
int Client::SendWholeMess()
{
	if (!ConfigurateMessage())
		return 0;

	int send_result = SendHeader();
	if (send_result != 1)
		return send_result;

	int sended_bytes = 0;
	while (sended_bytes != connection_.get_packet_lenght())
	{
		int chunk_size = connection_.TakeChunk();
		send_result = SendMess(chunk_size);

		if (send_result != 1)
			return send_result;

		sended_bytes += chunk_size;
	}

	return 1;
}

// returns:
// false - if client want to stop program.
// true - otherwise
bool Client::ConfigurateMessage()
{
	std::cout << "Write 0 - to send a message, 1 - to send a file, 2 - to exit"<< endl;
	char type;
	cin >> type;

	switch (type)
	{
	case '0':
	case '1':
		connection_.CreateMessage(int(type - '0'));
		break;
	case '2':
		return false;
	default:
		std::cout << "You are allowed to write only 0, 1 or 2! " << endl;
		cin.ignore(INT_MAX, '\n');
		return ConfigurateMessage();
	}

	if (connection_.ClarifyMessageSize() < 0)
		return ConfigurateMessage();

	connection_.CreateHeader();
	return true;
}

bool Client::CreateAddrinfo(char* host)
{
	bool returned_val = true;
	addrinfo hint;

	ZeroMemory(&hint, sizeof(hint));
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = IPPROTO_TCP;


	if (getaddrinfo(host, PORT, &hint, &res_))
	{
		cerr << "getadrrinfo failed with an error: "
			<< (int)WSAGetLastError() << endl;

		returned_val = false;
	}

	return returned_val;
}

bool Client::EstablishConnection()
{
	for (addrinfo* ptr = res_; ptr != NULL; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to server
		connection_.set_sock_(
			socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol));

		if (connection_.get_sock_() == INVALID_SOCKET)
		{
			cerr << "Ñreation of socket failed with an error: "
				<< (int)WSAGetLastError() << endl;

			return false;
		}

		// Connect to server.
		int iResult = connect(connection_.get_sock_(), ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult != SOCKET_ERROR)
		{
			// we connected to the server -> it's not necessary 
			// to check whether alternative addrinfo is valid
			break;
		}
		else
		{
			// if we are not able to connect with IPv6 -> 
			// check the same with IPv4
			closesocket(connection_.get_sock_());
			connection_.set_sock_( INVALID_SOCKET);
		}
	}

	// checking whether we complete the loop with break
	// statement or with its exit condition
	if (connection_.get_sock_() == INVALID_SOCKET) {
		cerr << "Unable to connect to the server!" << endl;
		return false;
	}
	else
	{
		connection_.connected();
		return true;
	}
}

bool Client::ShutdownConnection()
{
	bool returned_value = true;
	int result = shutdown(connection_.get_sock_(), SD_SEND);
	if (result == SOCKET_ERROR)
	{
		cerr << "shutdown failed with an error: "
			<< (int)WSAGetLastError() << endl;
		returned_value = false;
	}

	return returned_value;
}

bool Client::Clean()
{
	bool returned_value = true;
	connection_.disconnected();
	// res_ must be deleted earlier in the program.
	freeaddrinfo(res_);

	if (closesocket(connection_.get_sock_()) == SOCKET_ERROR)
	{
		cerr << "closesocket failed with an error: "
			<< (int)WSAGetLastError() << endl;
		returned_value = false;
	}

	return returned_value;
}
