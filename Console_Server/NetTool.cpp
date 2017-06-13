#include "NetTool.h"

SOCKET getListenSocket(char *port)
{
	WSADATA wsaData = { 0 };
	SOCKET ListenSocket = INVALID_SOCKET;
	int iResult;
	struct addrinfo *result = NULL;
	struct addrinfo hints;

	// 初始化WSA
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return INVALID_SOCKET;
	}

	// 初始化addrinfo结构
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_IP;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, port, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed!\n");
		return INVALID_SOCKET;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("Create socket failed!\n");
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind socket failed!\n");
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, NULL);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}

	return ListenSocket;
}

bool HandleIO(pTHREAD_OBJ pThread, int nIndex)
{
	WSANETWORKEVENTS NetWorks;
	SOCKET s = pThread->sockArray[nIndex - WSA_WAIT_EVENT_0];
	if (WSAEnumNetworkEvents(pThread->sockArray[nIndex - WSA_WAIT_EVENT_0], pThread->
		eventArray[nIndex - WSA_WAIT_EVENT_0], &NetWorks) == SOCKET_ERROR)
	{
		printf("WSAEnumNetworkEvents failed!\n");
		return false;
	}

	if (NetWorks.lNetworkEvents & FD_ACCEPT)
	{
		if (NetWorks.iErrorCode[FD_ACCEPT_BIT] != 0)
		{
			printf("FD_ACCEPT failed with error %d\n", NetWorks.iErrorCode[FD_ACCEPT_BIT]);
		}
		SOCKET client = accept(s, NULL, NULL);
		Client_info *clientinfo = PostAccept(client);
		int socketnum = (int)clientinfo->getSokcet();
		std::string str_name = std::string(clientinfo->getName());
		if (client != INVALID_SOCKET && clientinfo != NULL)
		{
			printf("new client accept!\n");
		}
		else
		{
			printf("new client add failed!\n");
			closesocket(client);
			return false;
		}
		WSAEVENT event = pThread->eventArray[pThread->dwEventCount] = WSACreateEvent();
		pThread->sockArray[pThread->dwEventCount] = client;
		int iResult = WSAEventSelect(client, event, FD_READ | FD_WRITE | FD_CLOSE);
		if (iResult != SOCKET_ERROR)
		{			
			printf("new client add to array succeed. Client Socket id:%d, Client name: %s!\n", socketnum, clientinfo->getName());
		}
		pThread->clientinfo[pThread->dwEventCount] = clientinfo;
		pThread->dwEventCount++;
		
		return true;
	}

	if (NetWorks.lNetworkEvents & FD_READ)
	{
		if (NetWorks.iErrorCode[FD_READ_BIT] != 0)
		{
			printf("FD_READ failed with error %d\n", NetWorks.iErrorCode[FD_READ_BIT]);
		}
		char recvbuff[64];
		std::string str;
		int iResult = recv(s, recvbuff, sizeof(recvbuff), 0);
		if (iResult > 0)
		{
			recvbuff[iResult] = '\0';
			str = std::string(recvbuff);
			printf("The client %s said %s\n", pThread->clientinfo[nIndex - WSA_WAIT_EVENT_0]->getName()
				, recvbuff);
		}
		if (str.find("play") != std::string::npos)
		{
			std::cout << "The client " << pThread->clientinfo[nIndex - WSA_WAIT_EVENT_0]->getName()
				<< " want a match!\n";
			pThread->clientinfo[nIndex - WSA_WAIT_EVENT_0]->setStatus(1);
			Match_info *match = wantplay(*(pThread->clientinfo[nIndex - WSA_WAIT_EVENT_0]), pThread);
			if (match == NULL)
			{
				return false;
			}
			pThread->pMatch = (pMATCH_OBJ)malloc(sizeof(Match_OBJ));
			pThread->pMatch->match = match;
			postmatch(match, pThread);
			pThread->pMatch->match_handle = CreateThread(NULL, 0, MatchThread, pThread->pMatch, 0, NULL);
			
		}
		return true;
	}
	return false;
}

Client_info* PostAccept(SOCKET s)
{
	char *hello = "Hello Client!";
	char welcome[64] = "Welcome ";
	char recvbuff[32];
	int iResult;
	iResult = send(s, hello, strlen(hello), NULL);
	int sec = 0;
	while (true)
	{
		iResult = recv(s, recvbuff, 32, NULL);
		if (iResult > 0)
		{
			recvbuff[iResult] = '\0';
			strcat(welcome, recvbuff);
			send(s, welcome, strlen(welcome), NULL);
			break;
		}
		else
		{
			if (++sec == 10)
			{
				return NULL;
			}
			printf("waiting client to response...\n");
			Sleep(1000);
		}
	}
	
	Client_info *clientinfo = new Client_info();
	clientinfo->setName(recvbuff);
	clientinfo->setStatus(0);
	clientinfo->setSocket(s);

	return clientinfo;
}

DWORD WINAPI EventThread(LPVOID lpParam)
{
	pTHREAD_OBJ pThread = (pTHREAD_OBJ)lpParam;

	while (true)
	{
		int nIndex = WSAWaitForMultipleEvents(pThread->dwEventCount, pThread->eventArray, FALSE, 5 * 1000, FALSE);
		if (nIndex == WSA_WAIT_FAILED)
		{
			printf("WSAWaitForMultipleEvents failed!\n");
			return -1;
		}
		if (nIndex == WSA_WAIT_TIMEOUT)
		{
			printf("Total Connection: %d\n", pThread->dwEventCount);
			for (int i = 1; i < pThread->dwEventCount; i++)
			{
				printf("\tClient Name: %s\tStatus: %d\n", pThread->clientinfo[i]->getName(), pThread->clientinfo[i]->getStatus());
			}
			continue;
		} 
		else
		{
			HandleIO(pThread, nIndex);
		}
	}
}

DWORD WINAPI MatchThread(LPVOID lpParam)
{
	fd_set fdread;
	fd_set fdwrite;
	struct timeval tv = { 1, 0 };
	
	pMATCH_OBJ pMatch = (pMATCH_OBJ)lpParam;
	Match_info *match = pMatch->match;
	Client_info *p1 = match->getP1();
	Client_info *p2 = match->getP2();
	

	printf("New Match has started!\n");
	printf("Player 1: %s\tPlayer 2: %s", p1->getName(), p2->getName());
	while (true)
	{
		FD_ZERO(&fdread);
		FD_SET(p1->getSokcet(), &fdread);
		FD_SET(p2->getSokcet(), &fdread);

		// only care about read event
		int nRet;
		nRet = select(NULL, &fdread, NULL, NULL, &tv);
		if (nRet == 0)
		{
			// Time expired and no events come
			printf("Matching ...\n");
			continue;
		}

		if (FD_ISSET(p1->getSokcet(), &fdread))
		{
			char recvbuf[64];
			nRet = recv(p1->getSokcet(), recvbuf, sizeof(recvbuf), NULL);

			if (nRet > 0)
			{
				recvbuf[nRet] = '\0';
				printf("Client %s: %s", p1->getName(), recvbuf);
				send(p1->getSokcet(), recvbuf, strlen(recvbuf), 0);
				send(p2->getSokcet(), recvbuf, strlen(recvbuf), 0);
			}
			else
			{
				printf("Client %s left the game!\n", p1->getName());
				break;
			}
		}

		if (FD_ISSET(p2->getSokcet(), &fdread))
		{
			char recvbuf[64];
			nRet = recv(p2->getSokcet(), recvbuf, sizeof(recvbuf), NULL);
			
			if (nRet > 0)
			{
				recvbuf[nRet] = '\0';
				printf("Client %s: %s", p2->getName(), recvbuf);
				send(p1->getSokcet(), recvbuf, strlen(recvbuf), 0);
				send(p2->getSokcet(), recvbuf, strlen(recvbuf), 0);
			}
			else
			{
				printf("Client %s left the game!\n", p2->getName());
				break;
			}
		}

	}

}

Match_info* wantplay(Client_info &c, pTHREAD_OBJ pThread)
{
	Client_info *client_p1 = &c;
	Client_info *client_p2;
	send(client_p1->getSokcet(), "WHO", strlen("WHO"), NULL);
	char recvbuff[32];
	char play[32] = "play";
	int sec = 0;
	while (true)
	{
		int iResult = recv(client_p1->getSokcet(), recvbuff, 32, NULL);
		if (iResult > 0)
		{
			recvbuff[iResult] = '\0';
			for (int i = 1; i < pThread->dwEventCount; i++)
			{
				if (strcmp(pThread->clientinfo[i]->getName(), recvbuff) == 0 && pThread->clientinfo[i]->getStatus() == 0)
				{
					client_p2 = pThread->clientinfo[i];
					break;
				}
			}
			break;
		}
		else
		{
			if (++sec == 10)
			{
				return NULL;
			}
			printf("waiting client to response(tell player)...\n");
			Sleep(1000);
		}
	}
	strcat(play, client_p1->getName());
	send(client_p2->getSokcet(), play, strlen(play), NULL);
	sec = 0;
	ZeroMemory(recvbuff, 32);
	while (true)
	{
		int iResult = recv(client_p2->getSokcet(), recvbuff, 32, NULL);
		if (iResult > 0)
		{
			recvbuff[iResult] = '\0';
			if (std::string(recvbuff) == "OK")
			{
				std::cout << "Player " + std::string(client_p2->getName()) + " OK!" << std::endl;
				client_p2->setStatus(1);
				break;
			}
			else
			{
				send(client_p1->getSokcet(), "NO", 2, NULL);
			}
		}
		else
		{
			if (++sec == 10)
			{
				return NULL;
			}
			printf("waiting client to response(wait player confirm)...\n");
			Sleep(1000);
		}
	}
	send(client_p1->getSokcet(), "OK", 2, NULL);
	Match_info *match = new Match_info();
	match->setP1(client_p1);
	match->setP2(client_p2);
	return match;
}

bool postmatch(Match_info *match, pTHREAD_OBJ pThread)
{
	SOCKET s1 = match->getP1()->getSokcet();
	SOCKET s2 = match->getP2()->getSokcet();

	WSAEventSelect(s1, getEventofSocket(s1, pThread), FD_CLOSE);
	WSAEventSelect(s2, getEventofSocket(s2, pThread), FD_CLOSE);
	return true;
}

WSAEVENT getEventofSocket(SOCKET s, pTHREAD_OBJ pThread)
{
	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; i++)
	{
		if (s == pThread->sockArray[i])
		{
			return pThread->eventArray[i];
		}
	}
	return NULL;
}
