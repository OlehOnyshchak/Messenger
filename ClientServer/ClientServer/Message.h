#pragma once
#ifndef CLIENTSERVER_CONNECTION__HEADERMANAGER_PROGRAM_CONNECTION_HEADERMANAGER_H_
#define CLIENTSERVER_CONNECTION__HEADERMANAGER_PROGRAM_CONNECTION_HEADERMANAGER_H_
#include "Connection.h"
#include <iostream>

// implements command version of message. In this case, command from console would be
// be proceeded or incoming command message would be saved there
class ClientServer::Connection::Command : public Message
{
public:
	int ClarifyMessageSize() override;
	
	void Flush(int) override;

	int TakeChunk() override;

	inline void ReleaseBuffResources() override
	{
		// nothing was acquired by Command
	}

	inline char* get_buff_() override
	{
		return buff_;
	}

	inline unsigned get_buff_size_() override
	{
		return COMMAND_BUFF_LEN;
	}
	
	inline void set_repository(std::string val) override
	{
		whole_command_ = val;
	}

private:
	char buff_[COMMAND_BUFF_LEN + 1];
	std::string whole_command_;
};

// implements file version of message. In this case, file from specified direction would be
// be proceeded or incoming file message would be saved there
class ClientServer::Connection::FileTrans : public Message
{
public:
	// takes client's ip address as parametr 
	FileTrans(char*);

	int ClarifyMessageSize() override;

	int TakeChunk() override;

	void Flush(int) override;

	inline void ReleaseBuffResources() override
	{
		std::cout << "File was transmitted" << std::endl;
		if (file_.is_open())
			file_.close();
	}

	inline char* get_buff_() override
	{
		return buff_;
	}

	inline unsigned get_buff_size_() override
	{
		return FILE_BUFF_LEN;
	}

	inline void set_repository(std::string val) override
	{
		path = val;
	}
private:
	std::fstream file_;
	std::string path = "";
	char buff_[FILE_BUFF_LEN + 1];
	char client_ip_[MAX_IP_SIZE + 1];
};

#endif