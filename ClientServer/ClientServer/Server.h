#pragma once
#ifdef _WIN32
#ifndef ClientServer_PROGRAM_SERVER_H_
#define ClientServer_PROGRAM_SERVER_H_

#include "ClientServer.h"
#include "Connection.h"

// Server which is used for incoming processing client's reques 
// and giving him a reply on each
class Server : public ClientServer
{
public:
	Server() = default;
	~Server();

	bool Work(char* port, char* ip) override;
private:
	// sending header with information, which is already in buffer 
	// and then send previously recorded message holded
	bool SendWholeMess(Connection& );

	bool ConfigurateSocket(char* port, char* ip = "localhost") override;
	bool CreateAddrinfo(char* port);
	bool CreateSocket();
	bool BindSocket();
	bool ListenRequest();
	bool AcceptRequest(Connection&);

	// function for separating work between new client and server's thread
	void NewRequestHandler(Connection&);

	// releasing an acquired data
	bool ShutdownSocket(Connection&);

private:
	// socket for listening all incoming requests
	SOCKET listening_;
};


#endif // !MY_SERVER_HEADER
#endif