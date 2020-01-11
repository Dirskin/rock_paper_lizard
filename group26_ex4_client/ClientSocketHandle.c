
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "../shared/socket_shared.h"
#include "../shared/SocketSendRecvTools.h"


SOCKET m_socket;
void MainClient();
void closed_connection(int ip, int port);

//Reading data coming from the server
static DWORD RecvDataThread(int ip, int port)
{
	TransferResult_t RecvRes;

	while (1)
	{
		char *AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, m_socket);

		if (RecvRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		else if (RecvRes == TRNS_DISCONNECTED){
			closed_connection(ip,port);
			return;
		}
		else
		{
			printf("%s\n", AcceptedStr);
		}

		free(AcceptedStr);
	}

	return 0;
}


//Sending data to the server
static DWORD SendDataThread(void)
{
	char SendStr[256];
	TransferResult_t SendRes;

	while (1)
	{
		gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard

		if (STRINGS_ARE_EQUAL(SendStr, "quit"))
			return 0x555; //"quit" signals an exit from the client side

		SendRes = SendString(SendStr, m_socket);

		if (SendRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
	}
}

void failed_connection(int ip, int port) {
	int decision;
	printf("Failed connecting to server on <%d>:<%d>\n", ip, port);
	printf("Choose what to do next:\n1. Try to reconnect\n2. Exit\n");
	while (1) {
		scanf_s("%d", &decision);
		if (decision == 1) { //chooses to try to reconnect
			MainClient();
		}
		else if (decision == 2) { //chooses to exit
			WSACleanup();
			return;
		}
		else {
			printf("please enter a valid answer\n");
		}
		
	}
}

void closed_connection(int ip, int port) {
	int decision;
	printf("Connection to server on <%d>:<%d> has been lost.\n", ip, port);
	printf("Choose what to do next:\n1. Try to reconnect\n2. Exit\n");
	while (1) {
		scanf_s("%d", &decision);
		if (decision == 1) { //chooses to try to reconnect
			MainClient();
		}
		else if (decision == 2) { //chooses to exit
			WSACleanup();
			return;
		}
		else {
			printf("please enter a valid answer\n");
		}
	}
 
}

void MainClient()
{
	SOCKADDR_IN clientService;
	HANDLE hThread[2];

	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()\n");


	// Create a socket.
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR); //Setting the IP address to connect to
	clientService.sin_port = htons(SERVER_PORT); //Setting the port to connect to.
	// Check for general errors.
	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		failed_connection(inet_addr(SERVER_ADDRESS_STR), htons(SERVER_PORT));
		return;
		}
	hThread[0] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)SendDataThread,
		NULL,
		0,
		NULL
	);
	hThread[1] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvDataThread(inet_addr(SERVER_ADDRESS_STR), htons(SERVER_PORT)),
		NULL,
		0,
		NULL
	);

	WaitForMultipleObjects(2, hThread, FALSE, INFINITE);

	TerminateThread(hThread[0], 0x555);
	TerminateThread(hThread[1], 0x555);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

	closesocket(m_socket);

	WSACleanup();

	return;
}
