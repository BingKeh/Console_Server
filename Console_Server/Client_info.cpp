#include "Client_info.h"

Client_info::Client_info()
{
}


Client_info::~Client_info()
{
}

void Client_info::setName(char *name)
{
	std::string str_name = std::string(name);
	int i;
	for (i = 0; i < str_name.length(); i++)
	{
		this->ClientName[i] = str_name[i];
	}
	this->ClientName[i] = '\0';
}

void Client_info::setStatus(int flag)
{
	this->ClientStatus = flag;
}

void Client_info::setSocket(SOCKET s)
{
	this->ClientSocket = s;
}

char* Client_info::getName()
{
	return this->ClientName;
}

int Client_info::getStatus()
{
	return this->ClientStatus;
}

SOCKET Client_info::getSokcet()
{
	return this->ClientSocket;
}
