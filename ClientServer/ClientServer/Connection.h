#pragma once
#ifndef ClientServer_CONNECTION_PROGRAM_ClientServer_H_
#define ClientServer_CONNECTION_PROGRAM_ClientServer_H_
#include "ClientServer.h"
#include <memory>
#include <utility>
#include <fstream>

// It will be used for representing whole separete connection
// with its all atributes
class ClientServer::Connection
{
public:
	// =====================================================================================
	// HeaderManager access methods
	
	// recodring header appropriated to "lenght" and "type" in header's buffer
	inline const char* CreateHeader()
	{
		return header_.CreateHeader(get_packet_lenght(), get_type());
	}

	// parsing incoming header from "sock_"
	inline std::pair<short, int> ReadHeader()
	{
		std::pair<short, int> properties = header_.ReadHeader(sock_);

		// each time, when we read new header - configurating new message type
		CreateMessage(properties.first, properties.second);

		return properties;
	}

	inline const char* get_header_()
	{
		return header_.get_header_();
	}

	inline int get_header_size()
	{
		return header_.get_header_size();
	}
	// ======================================================================================

	// ======================================================================================
	// Message access methods 

	inline int ClarifyMessageSize()
	{
		return mess_.get()->ClarifyMessageSize();
	}

	inline void ReleaseBuffResources()
	{
		return mess_.get()->ReleaseBuffResources();
	}

	inline int get_type()
	{
		return mess_.get()->get_type_();
	}

	// returned size of entire separete message
	inline int get_packet_lenght()
	{
		return mess_.get()->get_packet_lenght_();
	}

	inline char* get_mess()
	{
		return mess_.get()->get_buff_();
	}

	inline unsigned get_buff_size()
	{
		return mess_.get()->get_buff_size_();
	}

	// Flushing int first bytes from buffer in appropriate physical layer
	inline void Flush(int val)
	{
		mess_.get()->Flush(val);
	}

	// returns logically next piese of message
	inline int TakeChunk()
	{
		return mess_.get()->TakeChunk();
	}

	// changing information in message repository with appropriate message type/
	// It's a repository where whole message is saving, whether it is a Command.
	// Otherwise, it's a repositore, where all transmitted fyles are saving
	inline void set_repository(std::string val, short type)
	{
		CreateMessage(type, val.size());
		mess_.get()->set_repository(val);
	}
	// ======================================================================================

	// ======================================================================================
	// Connection fields access methods 

	// creating or changing Message implementation into desirable type.
	// if message's lenght is impossible to indicate at moment of call, 
	// it must be done before sending or creating header or message
	void CreateMessage(short type, const int& size = -1);

	// indicates whether there is a valid connection
	inline bool get_is_connected_()
	{
		return is_connected_;
	}

	// informing that connection is successfully established
	inline void connected()
	{
		is_connected_ = true;
	}

	// infroming that socket is disconnected
	inline void disconnected()
	{
		is_connected_ = false;
	}

	inline SOCKET get_sock_()
	{
		return sock_;
	}

	inline void set_sock_(SOCKET val)
	{
		sock_ = val;
	}
	// ======================================================================================

	// ======================================================================================
	// ConnectIp access methods 

	inline char* get_clients_ip()
	{
		return address_.get_addr_();
	}

	inline void record_id(sockaddr_storage client)
	{
		address_.record_id(client);
	}
	// ======================================================================================


private:
	// indicates whether memory for holding message information was allocated
	inline bool mess_is_init()
	{
		return mess_.get() != nullptr;
	}


private:
	//=================================================================================
	//=================================================================================
	// Manage creating, receiving and parsing packet's header
	class HeaderManager
	{
	public:
		// it would create a header, where first HEADER-1 bits is bynary formatted size value
		// and the HEADER-th bit indicate that file'll be sending, if it equals 1. Otherwise - it'll 
		// be a pure message. The last HEADER+1 bit - is a HEADER_TERMINATOR
		const char* CreateHeader(const unsigned& lenght, const short& is_file);

		// reads a header using parameter socket and parsing it
		std::pair<short, int> ReadHeader(SOCKET& connected);

		inline char* get_header_()
		{
			return header_;
		}

		inline int get_header_size()
		{
			return TOTAL_HEADER_SIZE;
		}
	private:
		// checking wheather C-style string has header-terminator symbol
		bool HasHeaderTerminator();

		void EraseTerminator();

		// extracts from header amount of packet's bytes and 
		// writes in is_a_file whether is is file (true) or 
		// pure message (false)
		std::pair<short, int> ParseHeader();

	private:
		char header_[TOTAL_HEADER_SIZE + 1];
	};
	//=================================================================================
	//=================================================================================


	//=================================================================================
	//=================================================================================
	// Hold whole information about message such as : its size, type and message itself
	class Message
	{
	public:
		// clarifying message's size and from where it will be read.
		// In case of Command message, it will be placed in repository 
		// by this call. Otherwise, just clarification from where file
		// will be readed is done
		virtual int ClarifyMessageSize() = 0;

		// returns logically next piese of message
		virtual int TakeChunk() = 0;

		virtual char* get_buff_() = 0;

		virtual unsigned get_buff_size_() = 0;

		virtual void Flush(int) = 0;

		virtual void ReleaseBuffResources() = 0;
	
		virtual void set_repository(std::string) = 0;

		// it returns unique message number, when it is needed
		static unsigned GetId();

		inline void set_packet_lenght_(int val)
		{
			packet_lenght_ = val;
		}

		inline void set_type_(short val)
		{
			packet_type_ = val;
		}

		inline int get_packet_lenght_()
		{
			return packet_lenght_;
		}

		inline int get_type_()
		{
			return packet_type_;
		}

		inline int get_sended_bytes_()
		{
			return sended_bytes_;
		}
	protected:
		int packet_lenght_ = -1;
		short packet_type_ = -1;
		// don forget to set it to null after sending
		int sended_bytes_ = 0;
		static unsigned id_;
	};

	// Classes are derived from Message
	class Command;
	class FileTrans;
	//=================================================================================
	//=================================================================================

	
	//=================================================================================
	//=================================================================================
	// hold information about client's ip and proceeding it
	class ConnectedIp
	{
	public:
		inline char* get_addr_()
		{
			return addr_;
		}

		// transforming client's id from network form and 
		// write it in buffer
		inline void record_id(sockaddr_storage clients)
		{
			inet_ntop(clients.ss_family,
				get_in_addr((sockaddr *)&clients),
				addr_, MAX_IP_SIZE + 1);
		}
	private:
		// casts sockaddr into IPv6 or IPv4 appropriatedly  to what sockaddr is
		void* get_in_addr(sockaddr *sa)
		{
			if (sa->sa_family == AF_INET)
			{
				return &(((sockaddr_in*)sa)->sin_addr);
			}

			return &(((sockaddr_in6*)sa)->sin6_addr);
		}
	private:
		char addr_[MAX_IP_SIZE + 1];
	};
	//=================================================================================
	//=================================================================================

private:
	HeaderManager header_;
	ConnectedIp address_;
	SOCKET sock_;
	// if it has a valid connection
	bool is_connected_;
	std::unique_ptr<Message> mess_;
};

//unsigned ClientServer::Connection::id_ = 0;

#endif