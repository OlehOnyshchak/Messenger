#ifdef _WIN32
#include "ClientServer.h"
#include "Connection.h"
#include <iostream>


using std::cerr;
using std::endl;
using std::cout;


// input: C-style string message, which will be transmitted
// returns:
// 1 - if sending was successful 
// SOCKET_ERROR -  if sending failed.
// ESTABLISHING_CONNECTION_ERROR - if socket is not connected
int ClientServer::SendMess(Connection& connection, int sending_bytes)
{
	int returned_value = 1;
	if (connection.get_is_connected_())
	{
		if (connection.get_packet_lenght() > 0)
		{
			int send_result = NakedSend(connection.get_mess(), sending_bytes, connection);
			if (send_result != 1)
				returned_value = send_result;
		}
	}
	else
	{
		cerr << "Send ERROR: you haven't establish a connection!" << endl;
		returned_value = ESTABLISHING_CONNECTION_ERROR;
	}

	return returned_value;
}


// returns:
// 1 - if sending was successful 
// SOCKET_ERROR -  if sending failed.
// ESTABLISHING_CONNECTION_ERROR - if socket is not connected
int ClientServer::SendHeader(Connection& connection)
{
	int returned_value = 1;
	if (connection.get_is_connected_())
	{
		int send_result = NakedSend(connection.get_header_(), connection.get_header_size(), connection);
		if (send_result != 1)
			returned_value = send_result;
	}
	else
	{
		cerr << "Send ERROR: you haven't establish a connection!" << endl;
		returned_value = ESTABLISHING_CONNECTION_ERROR;
	}

	return returned_value;
}

// Pre-condition: connection must be previously established
// input: C-style string message and amount of bytes, which will be sended
// will return 1 , if complete successfully;
// otherwise will return SOCKET_ERROR
int ClientServer::NakedSend(const char* mess, const unsigned& lenght, Connection& connection)
{
	int sended_bytes = 0;
	while (sended_bytes != lenght)
	{
		// at this point an error occured on 2 iteration, when sending huge file from client
		// perhaps, there is some problem in server's side
		int send_res = send(connection.get_sock_(), mess + sended_bytes, lenght - sended_bytes, 0);
		if (send_res == SOCKET_ERROR)
		{
			std::cerr << "send failed with an error: "
				<< WSAGetLastError() << endl;
			return SOCKET_ERROR;
		}

		sended_bytes += send_res;
	}

	return 1;
}



// output:
// 1 - if message was received
// 2 - if file was received
// CONNECTION_GRACEFFULLY_CLOSED - connection closed
// ESTABLISHING_CONNECTION_ERROR - socket not connected
// SOCKET_ERROR - an error occured
int ClientServer::RecvMess(Connection& connect)
{
	short message_type;
	if (!connect.get_is_connected_())
	{
		cerr << "Recv ERROR: you haven't establish a connection!" << endl;
		return ESTABLISHING_CONNECTION_ERROR;
	}
	else
	{
		std::pair<short, int> parsed_header = connect.ReadHeader();
		int readed_lenght = parsed_header.second;

		if (readed_lenght == 0 || readed_lenght == SOCKET_ERROR)
			return readed_lenght;

		// 1 - if message, 2 - if file
		// but originally it was extracted as 0 and 1 respectively
		// from header
		message_type = parsed_header.first + 1;

		int recv_bytes = 0;
		// waiting for data untill full packet would be readed
		while (recv_bytes != connect.get_packet_lenght())
		{
			// rewrite inforamion in buffer from the beginning because
			// in this program version buffer was proceeded at the beginning of the iteration
			int buff_size = connect.get_buff_size();
			if ((connect.get_packet_lenght() - recv_bytes) < buff_size)
				buff_size = connect.get_packet_lenght() - recv_bytes;

			int recv_res = recv(connect.get_sock_(), connect.get_mess(), buff_size, 0);
			switch (recv_res)
			{
			case CONNECTION_GRACEFFULLY_CLOSED:
				cout << "Current connection closing... some data was losted!" << endl << endl;
				return CONNECTION_GRACEFFULLY_CLOSED;

			case SOCKET_ERROR:
				cerr << "Receiving failed... some data was losted!" << endl;
				return recv_res;

			default:
				connect.Flush(recv_res);
				recv_bytes += recv_res;
				break;
			}
		}// end of while-loop
		connect.ReleaseBuffResources();

		return message_type;
	}
}

#endif