#pragma once

#include <WinSock2.h>
#include <string>

class Client_info
{
private:
	SOCKET ClientSocket;
	char ClientName[32];
	int ClientStatus;

public:
	Client_info();
	~Client_info();
	void setName(char *name);
	void setStatus(int flag);
	void setSocket(SOCKET s);
	char* getName();
	int getStatus();
	SOCKET getSokcet();
};

