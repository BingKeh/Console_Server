#pragma once

#include <WinSock2.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <ws2tcpip.h>
#include <stdlib.h>
#include "Match_info.h"

#pragma comment (lib, "Ws2_32.lib")

typedef struct _Match_OBJ
{
	HANDLE match_handle;
	Match_info *match;
} Match_OBJ, *pMATCH_OBJ;

typedef struct _Thread_OBJ
{
	WSAEVENT eventArray[WSA_MAXIMUM_WAIT_EVENTS];
	SOCKET sockArray[WSA_MAXIMUM_WAIT_EVENTS];
	DWORD dwEventCount;
	Client_info *clientinfo[WSA_MAXIMUM_WAIT_EVENTS];
	pMATCH_OBJ pMatch;
} Thread_OBJ, *pTHREAD_OBJ;



SOCKET getListenSocket(char *port);
bool HandleIO(pTHREAD_OBJ pThread, int nIndex);
Client_info* PostAccept(SOCKET s);
DWORD WINAPI EventThread(LPVOID lpParam);
DWORD WINAPI MatchThread(LPVOID lpParam);
Match_info* wantplay(Client_info &c, pTHREAD_OBJ pThread);
bool postmatch(Match_info *match, pTHREAD_OBJ pThread);
WSAEVENT getEventofSocket(SOCKET s, pTHREAD_OBJ pThread);