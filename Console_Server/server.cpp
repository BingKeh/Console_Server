#include "NetTool.h"

int main(void)
{
	WSAEVENT event;

	SOCKET Listensocket;
	Listensocket = getListenSocket("2333");
	if (Listensocket != INVALID_SOCKET)
	{
		printf("getListenSocket succeed!\n");
	}

	event = WSACreateEvent();

	int iResult = WSAEventSelect(Listensocket, event, FD_ACCEPT);
	if (iResult != SOCKET_ERROR)
	{
		printf("WSAEventSelect succeed!\n");
	}
	pTHREAD_OBJ pThread = (pTHREAD_OBJ)malloc(sizeof(Thread_OBJ));
	ZeroMemory(pThread, sizeof(Thread_OBJ));
	pThread->dwEventCount = 1;
	pThread->eventArray[0] = event;
	pThread->sockArray[0] = Listensocket;

	HANDLE handle = CreateThread(NULL, 0, EventThread, pThread, 0, NULL);
	WaitForSingleObject(handle, INFINITE);
	return 0;
}

