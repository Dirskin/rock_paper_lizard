
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include "clientsockethandle.h"
#include "../shared/socket_shared.h"
#include "../shared/SocketSendRecvTools.h"


SOCKET m_socket;

int failed_connection(const char *server_ip_addr, int port, int flag_type) {
	int decision;
	switch (flag_type) {
	case F_SERVER_CONNECTION_FAILED:
		printf("Failed connecting to server on <%s>:<%d>.\n", server_ip_addr, port);
		break;
	case F_SERVER_CONNECTION_LOST:
		printf("Connection to server on <%s>:<%d> has been lost.\n", server_ip_addr, port);
		break;
	case F_SERVER_DENIED_CONNECTION:
		printf("Server on <%s>:<%d> denied the connection request.\n", server_ip_addr, port);
		break;
	}

	printf("Choose what to do next:\n1. Try to reconnect\n2. Exit\n");
	while (1) {
		scanf_s("%d", &decision);
		if (decision == 1) { //chooses to try to reconnect
			printf("thanks for choosing 1\n");
			return 0;
		}
		else if (decision == 2) { //chooses to exit
			WSACleanup();
			return 0;
		}
		else {
			printf("please enter a valid answer\n");
		}

	}
}


//Reading data coming from the server
static DWORD RecvDataThread(const char *server_ip_addr, int port)
{
	TransferResult_t RecvRes;
	int err;

	while (1)
	{
		char *AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, m_socket);

		if (RecvRes == TRNS_FAILED) {
			err = failed_connection(server_ip_addr, port, F_SERVER_CONNECTION_LOST);
			return err;
		}
		else if (RecvRes == TRNS_DISCONNECTED) {
			err = failed_connection(server_ip_addr, port, F_SERVER_CONNECTION_LOST);
			return err;
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

int MainClient(const char *server_ip_addr, int server_port) {
	SOCKADDR_IN clientService;
	HANDLE hThread[2];
	int err;
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
		return ERR_SOCKET;
	}
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(server_ip_addr); //Setting the IP address to connect to
	clientService.sin_port = htons(server_port);			 //Setting the port to connect to.
	// Check for general errors.
	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		err = failed_connection(server_ip_addr, server_port, F_SERVER_CONNECTION_FAILED);
		return err;		/*Thats not an error: user chose to exit*/
		}
	printf("Connected to server on %s:%d\n", server_ip_addr, server_port);

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
		(LPTHREAD_START_ROUTINE)RecvDataThread(server_ip_addr, server_port),
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

	return 0;
}
