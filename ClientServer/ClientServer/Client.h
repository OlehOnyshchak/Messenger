#pragma once
#ifdef _WIN32

#ifndef ClientServer_PROGRAM_CLIENT_H_
#define ClientServer_PROGRAM_CLIENT_H_

#include "ClientServer.h"
#include "Connection.h"
#include <string>
#include <iostream>

using std::cerr;
using std::endl;
using std::cin;

// Client which is used for connection with known server with 
// purpose to sedn request and recieve reply on each of them
class Client : public ClientServer
{
public:
	Client() = default;
	~Client() = default;

	bool Work(char* host, char* ip = "localhost") override;
private:
	bool ConfigurateSocket(char* port, char* ip = "localhost") override;

	// function "packets" for ClientServer functions, which
	// bind them to the single Client's member conncetion
	int SendMess(int);
	int SendHeader();
	int RecvMess();

	// sending header and whole appropriate message
	int SendWholeMess();

	bool ConfigurateMessage();
	bool CreateAddrinfo(char*);
	bool EstablishConnection();
	bool ShutdownConnection();

	// releasing an acquired data
	bool Clean();
private:
	Connection connection_;
};


#endif // !MY_CLIENT_HEADER
#endif // !_WIN32