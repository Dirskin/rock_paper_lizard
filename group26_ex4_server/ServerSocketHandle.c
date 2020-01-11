#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <winsock2.h>
#include "../shared/socket_shared.h"
#include "../shared/SocketSendRecvTools.h"

#define NUM_OF_WORKER_THREADS 2
#define MAX_LOOPS 3
#define SEND_STR_SIZE 35
#define MAX_STDIN_ARG_SIZE 256

/* Globals */
HANDLE ThreadHandles[NUM_OF_WORKER_THREADS], check_exit_handle;
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
static int FindFirstUnusedThreadSlot();
static void CleanupWorkerThreads();
static DWORD ServiceThread(SOCKET *t_socket);
bool received_exit = false;

static DWORD CheckExit(void)
{
	char str_in[MAX_STDIN_ARG_SIZE];
	while (1) {
		scanf("%s", str_in);
		
		if (strcmp(str_in, "exit")==0) {
			printf("exiting, bye\n");
			received_exit = true;
			return 0;
		}
	}
}
HANDLE start_exit_thread() {
	HANDLE ret_handle;
	ret_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CheckExit, NULL, 0, NULL);
	return ret_handle;
}

void MainServer(int port)
{
	int Ind;
	int Loop;
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (StartupRes != NO_ERROR) {
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());                                
		return;
	}
 
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  /* Server side socket creation*/
	if (MainSocket == INVALID_SOCKET) {
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		goto server_cleanup_1;
	}

	// create SOCKADDR params and Bind the socket
	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE) {
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		goto server_cleanup_2;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address; /* TO CHECK!! --- "127.0.0.1" is the local IP address to which the socket will be bound.*/
	service.sin_port = htons(port); 

	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	check_exit_handle = start_exit_thread(); /*Run thread in background to check for "exit" in console*/

	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;

	printf("Waiting for a client to connect...\n");

	for (Loop = 0; Loop < MAX_LOOPS; Loop++)
	//while (!received_exit);
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			goto server_cleanup_3;
		}
		printf("Client Connected.\n");
		Ind = FindFirstUnusedThreadSlot();

		if (Ind == NUM_OF_WORKER_THREADS) //no slot is available
		{
			printf("No slots available for client, dropping the connection.\n");
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.
		}
		else {
			ThreadInputs[Ind] = AcceptSocket; // shallow copy: don't close, AcceptSocket, instead close, ThreadInputs[Ind] when the time comes.
			ThreadHandles[Ind] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServiceThread, &(ThreadInputs[Ind]), 0, NULL);
		}
	} // while
	/*need to add code to close thread better and to signal threads for finish.*/

server_cleanup_3:
	CleanupWorkerThreads();

server_cleanup_2:
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());

server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}

static int FindFirstUnusedThreadSlot()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], 0);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}

static void CleanupWorkerThreads()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] != NULL)
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], INFINITE);

			if (Res == WAIT_OBJECT_0)
			{
				closesocket(ThreadInputs[Ind]);
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
			else
			{
				printf("Waiting for thread failed. Ending program\n");
				return;
			}
		}
	}
}

//Service thread is the thread that opens for each successful client connection and "talks" to the client.
static DWORD ServiceThread(SOCKET *t_socket)
{
	char SendStr[SEND_STR_SIZE];

	BOOL Done = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;

	strcpy(SendStr, "Connected to server on <ip>:<port>"); //need to add apropriate ip and port
	SendRes = SendString(SendStr, *t_socket);

	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		closesocket(*t_socket);
		return 1;
	}

	while (!Done)
	{
		char *AcceptedStr = NULL;

		RecvRes = ReceiveString(&AcceptedStr, *t_socket);

		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			closesocket(*t_socket);
			return 1;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection closed while reading, closing thread.\n");
			closesocket(*t_socket);
			return 1;
		}
		else
		{
			printf("Got string : %s\n", AcceptedStr);
		}

		if (STRINGS_ARE_EQUAL(AcceptedStr, "hello"))
		{
			strcpy(SendStr, "what's up?");
		}
		else if (STRINGS_ARE_EQUAL(AcceptedStr, "how are you?"))
		{
			strcpy(SendStr, "great");
		}
		else if (STRINGS_ARE_EQUAL(AcceptedStr, "bye"))
		{
			strcpy(SendStr, "see ya!");
			Done = TRUE;
		}
		else
		{
			strcpy(SendStr, "I don't understand");
		}

		SendRes = SendString(SendStr, *t_socket);

		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			closesocket(*t_socket);
			return 1;
		}

		free(AcceptedStr);
	}
	printf("Conversation ended.\n");
	closesocket(*t_socket);
	return 0;
}
