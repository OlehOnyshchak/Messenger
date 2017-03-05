#pragma once
#ifdef _WIN32

#ifndef ClientServer_PROGRAM_ClientServer_H_
#define ClientServer_PROGRAM_ClientServer_H_

//////////////////////////////////////////////////////
// includes
#include <winsock2.h>
#include <ws2tcpip.h>


// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
//////////////////////////////////////////////////////

//=====================================================================
// defines
#define PORT "27015"

#define MY_WINSOCK_VERSION MAKEWORD(2,2)
#define ESTABLISHING_CONNECTION_ERROR -15
#define CONNECTION_GRACEFFULLY_CLOSED 0
#define MESSAGE_WAS_TRANSMITTED 1
#define FILE_WAS_TRANSMITTED 2
#define MESSAGE_TYPE 0
#define FILE_TYPE 1

#define MAX_FILE_DIRECTION_SIZE 50
#define COMMAND_BUFF_LEN 50
#define FILE_BUFF_LEN 1024
#define MAX_IP_SIZE 16
#define BITS_IN_BYTE 8
#define TYPE_INDICATOR_LENGHT 1
#define TERMINATOR_LENGHT 1
#define HEADER_TERMINATOR '@'

// indicates amount of bytes without header's terminating string 
#define SIGNIFICANT_HEADER_SIZE sizeof(unsigned) * BITS_IN_BYTE + TYPE_INDICATOR_LENGHT

// indicates amount of bytes including header's terminating string
#define TOTAL_HEADER_SIZE SIGNIFICANT_HEADER_SIZE + TERMINATOR_LENGHT

#define HEADER_BUFFER_SIZE HEADER_SIZE + TERMINATOR_LENGHT + 1

// interface for access to both regimes: client and server
// with implemetations of recive and send methods
class ClientServer
{
public:
	virtual ~ClientServer() = default;

	// Force both server and client act in orthodox way, that is
	// client is sending multiple requests and receive reply on each 
	// of them from server
	virtual bool Work(char* port, char* ip = "localhost") = 0;


protected:
	// declaration of internal class
	class Connection;

	// Establish connection with the apropriate server, if it is a client.
	// Otherwise binding to the apropriate port and then, listening it
	virtual bool ConfigurateSocket(char* port, char* ip = "localhost") = 0;

	// sending first "int" bytes from message's buffer via "Connection"
	int SendMess(Connection&, int);
	// sending header from header's buffer via "Connection"
	int SendHeader(Connection&);
	// sending message "const char*" with size "unsigned" via "Conection" 
	int NakedSend(const char* , const unsigned& , Connection& );

	// Reading exactly one separate packet. 
	int RecvMess(Connection&);

protected:
	// It will be used for representing whole separete connection
	// with its all atributes
	class Connection;
	
private:
	// Incapsulate all operations with WSAdata
	class MyWsaData
	{
	public:
		// libraries will be uploaded in construcor and set 
		// "uploaded_" to true if succeed
		MyWsaData();
		// releasing all acquired resourses
		~MyWsaData();

		inline bool get_uploaded_()
		{
			return uploaded_;
		}
	private:
		WSADATA wsa_data_;
		bool uploaded_;
	};
protected:
	// contain all information about the connection's type
	addrinfo *res_;

	// hold uploaded winsock library
	MyWsaData wsa_data_;
};

#endif
#endif