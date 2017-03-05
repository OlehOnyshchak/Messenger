#include "Message.h" 
#include <string>
#include <iostream>
#include <thread>         
#include <mutex>

using std::cout;
using std::cin;
using std::endl;
using std::ios;

unsigned ClientServer::Connection::Message::GetId()
{
	// protection from mutithereading
	std::mutex mtx;
	mtx.lock();
	unsigned val = ++id_;
	mtx.unlock();
	return val;
}

int ClientServer::Connection::Command::ClarifyMessageSize()
{
	cout << "Please, write your message" << endl;
	whole_command_ = "";
	// if newline character is lefted in buffer 
	// after cin's work - erasing it in odrer 
	// of getline correct work
	if(cin.peek() == '\n')
		cin.ignore();

	getline(cin, whole_command_);
	whole_command_ += "\n\0";

	packet_lenght_ = whole_command_.size();

	return packet_lenght_;
}

int ClientServer::Connection::Command::TakeChunk()
{
	// copying_size equals to buffer size, if amount of not
	// extracted bytes is bigger then buffer itself.
	// Otherwise, it equals exact size of lefted bytes
	int copying_size = get_buff_size_();
	if ((packet_lenght_ - sended_bytes_) < get_buff_size_())
		copying_size = packet_lenght_ - sended_bytes_;

	std::string chunk(whole_command_, sended_bytes_, copying_size);
	chunk += '\0';

	// copying next chunk in message's buffer
	strcpy_s(buff_, copying_size + 1, chunk.c_str());

	// counting amount of bytes which was given
	sended_bytes_ += copying_size;

	// it will set sended_bytes to null, when whole packet is send
	sended_bytes_ %=  packet_lenght_;

	return copying_size;
}

void ClientServer::Connection::Command::Flush(int size)
{
	buff_[size] = '\0';
	cout << buff_;
}
//===================================================================================


ClientServer::Connection::FileTrans::FileTrans(char * ip)
{
	strcpy_s(client_ip_, MAX_IP_SIZE + 1, ip);
}

int ClientServer::Connection::FileTrans::ClarifyMessageSize()
{
    cout << "Input file's directory" << endl;
	std::string path;
	cin >> path;
	path += '\0';

	file_.open(path.c_str(), ios::ate | ios::binary| ios::in);
	
	if (file_.is_open())
	{
		std::streampos end = file_.tellg();

		file_.seekg(0, ios::beg);
		std::streampos beg = file_.tellg();

		packet_lenght_ = end - beg;

		return packet_lenght_;
	}
	else
	{
		std::cerr << "Opening file error" << endl;
		return -1;
	}
}

int ClientServer::Connection::FileTrans::TakeChunk()
{
	int copying_size = get_buff_size_();
	if ((packet_lenght_ - sended_bytes_) < get_buff_size_())
		copying_size = packet_lenght_ - sended_bytes_;

	file_.read(buff_, copying_size);
	
	sended_bytes_ += copying_size;
	sended_bytes_ %= packet_lenght_;
	
	// it will equals true only and only full packet was send
	if (sended_bytes_ == 0 && copying_size != 0)
	{
		file_.close();
	}
	
	return copying_size;
}

void ClientServer::Connection::FileTrans::Flush(int size)
{
	if (!file_.is_open())
	{
		char name[MAX_FILE_DIRECTION_SIZE];
		// path will be added there
		// creating unique name for each transmitted file.
		// Also, clients ip, from where file was transmitted 
		// will be added to file name lately
		sprintf_s(name, 50,  "D:\\Client_%d", GetId());

		file_.open(name, ios::app | ios::binary | ios::out);
		cout << "File "<< name << " is transmitting...   " ;
	}

	file_.write(buff_, size);
}

void ClientServer::Connection::CreateMessage(short type, const int & size)
{
	// it will be true only when message type need to be changed.
	// Otherwise, there won't be reallocation, but only correctiong 
	// properties information
	if (!mess_is_init() || type != get_type())
	{
		if (type == 0)
		{
			mess_.reset(new Command);
		}
		else if (type == 1)
		{
			mess_.reset(new FileTrans(get_clients_ip()));
		}
		mess_.get()->set_type_(type);
	}
	mess_.get()->set_packet_lenght_(size);
}