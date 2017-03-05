#include "ClientServer.h"
#include "Connection.h"
#include <bitset>
#include <iostream>
#include <memory>
#include <utility>

using std::cerr;
using std::endl;
using std::cout;


// it would create a header, where first HEADER-1 bits is bynary formatted size value
// and the HEADER-th bit indicate that file'll be sending, if it equals 1. Otherwise - it'll 
// be a pure message. The last HEADER+1 bit - is a HEADER_TERMINATOR
const char* ClientServer::Connection::HeaderManager::CreateHeader(const unsigned  &lenght, const short &type)
{
	std::bitset<SIGNIFICANT_HEADER_SIZE> header(lenght);
	// now type can only might have 2 values. 
	// But if neccessary, its range will be easily extended to 4, 8, 16 etc.
	header.set(SIGNIFICANT_HEADER_SIZE - 1, type);

	std::string header_with_terminator = header.to_string() + HEADER_TERMINATOR;
	strcpy_s(header_, TOTAL_HEADER_SIZE + 1, header_with_terminator.c_str());

	return header_;
}

// assuming, that before calling ParseHeader the correct header
// was recorded in header_[]
// it will return the size of packet and write in packet_type_
// wheather it's file or not
// header must be with C-style null terminating character
// but without packet_header terminating character
std::pair<short,int> ClientServer::Connection::HeaderManager::ParseHeader()
{
	std::bitset<SIGNIFICANT_HEADER_SIZE> pars(header_);
	
	// extracting information about message's type
	short type = pars.test(SIGNIFICANT_HEADER_SIZE - 1);
	pars.set(SIGNIFICANT_HEADER_SIZE - 1, 0);

	// extracting information about header's size
	int size = static_cast<int>(pars.to_ulong());
	return 	std::make_pair(type, size);

}

// checks wheather C-style string has packet's HEADER_TERMINATOR
// will be changend on more reliable version futher
bool ClientServer::Connection::HeaderManager::HasHeaderTerminator()
{
	// temporary code with testing one-character terminator
	return (header_[SIGNIFICANT_HEADER_SIZE] == HEADER_TERMINATOR);
}


// waiting until some data will occur in the stream
// input:
// connected_ - socket from where the message is expected 
// output:
// positive integer - if receiving succeeded, indicates packet's lenght
// 0 - connection closed
// SOCKET_ERROR - an error occured
std::pair<short,int> ClientServer::Connection::HeaderManager::ReadHeader(SOCKET& sock)
{
	int recv_res = recv(sock, header_, TOTAL_HEADER_SIZE, 0);
	switch (recv_res)
	{
	case CONNECTION_GRACEFFULLY_CLOSED:
//		cout << "Current connection closing..." << endl << endl;
		return std::make_pair(0,0);

	case SOCKET_ERROR:
		cerr << "Recv ERROR: reaciving failed" << endl;
		return std::make_pair(0, SOCKET_ERROR);

	default:
		// Supposing, we read a packet header
		// putting terminating 0 character at the end of readed sequence
		header_[recv_res] = 0;
		break;
	}


	// checking wheather we accepted a complete header
	if (strlen(header_) != TOTAL_HEADER_SIZE ||
		!HasHeaderTerminator())
	{
		cerr << "Recv ERROR: header reaciving is failed" << endl;
		return std::make_pair(SOCKET_ERROR, 0);
	}
	else
	{
		// erasing header terminator in order to use ParseHeader
		EraseTerminator();
	}

	return ParseHeader();
}

void ClientServer::Connection::HeaderManager::EraseTerminator()
{
	// will be more reliable, when terminator become bigger then 1 byte
	header_[SIGNIFICANT_HEADER_SIZE] = 0;
}