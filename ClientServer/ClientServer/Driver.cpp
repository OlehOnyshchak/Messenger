#include "Server.h"
#include "Client.h"
#include <iostream>
#include <cctype>
#include <memory>
#include <algorithm>

using std::string;
using std::cout;
unsigned ClientServer::Connection::Message::id_ = 0;

ClientServer* program_regime()
{
	cout << "Please, write down whether you want to use" << endl <<
		"this program as a server or as a client" << endl;
	cout << R"(( Write "client" or "server" ))" << endl;
	string answer;
	cin >> answer;
	std::transform(answer.begin(), answer.end(), answer.begin(), ::tolower);

	ClientServer* prog;
	if (answer == "server")
	{
		cout << "Server regime is activated" << endl;
		prog = new Server;
		// estimating a newline character from the buffer

		return prog;
	}
	else if (answer == "client")
	{
		cout << "Client regime is activated" << endl;
		prog = new Client;
		return prog;
	}
	else
	{
		cout << "You wrote incorrect string! " << endl << endl;
		return program_regime();
	}
}

void main(int argc, char** argv)
{
	std::unique_ptr<ClientServer*> program = std::make_unique<ClientServer*>(program_regime());
	if (argc == 2)
	{
		(*program.get())->Work(PORT, argv[1]);
	}
	else if (argc == 1)
	{
		(*program.get())->Work(PORT);
	}
	system("pause");
}
