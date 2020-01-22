#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "../shared/socket_shared.h"
#include "../shared/SocketSendRecvTools.h"

#define NUM_OF_WORKER_THREADS 2
#include "thread_handle.h"
#include "gameplay.h"
#include "../shared/socket_shared.h"
#include "../shared/SocketSendRecvTools.h"


#define MAX_LOOPS 1024  /* originally 3.*/
#define SEND_STR_SIZE 35
#define MAX_STDIN_ARG_SIZE 256

/* Globals */
HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
bool received_exit = false;
bool game_status[2] = { false, false }; /*array to signal if there are two players who wishes to play against each other*/


/*Local/private Function Declarations*/
static DWORD ClientThread(int priv_index);

static DWORD ServiceThread() {
	HANDLE game_semp_handle = init_game_semp(); /* Semaphore initialized to ZERO */
	HANDLE file_mutex_handle = init_file_mutex(); /* creating global mutex to protect file*/
	DWORD wait_code, wait_code2;
	bool ret_val;
	bool was_game = false;
	int err, err2;
	if (game_semp_handle == NULL || file_mutex_handle == NULL) return ERR_MUTEX; /* Return error from service thread*/
	while (!received_exit) {
		if (!was_game && (game_status[0] == true && game_status[1] == true)) {
			err = ReleaseSemaphore(game_semp_handle, 1, NULL); /* Adds 2 slots for the semaphore*/
			err2 = ReleaseSemaphore(game_semp_handle, 1, NULL); /* Adds 2 slots for the semaphore*/
			if (err == FALSE || err2 == FALSE) {
				printf("Error releasing semaphore\n");
				return ERR;
			}
			was_game = true;
		}
		if (was_game && (game_status[0] == false && game_status[1] == false)) {	/*2 client finished playing, acquiring 2(!) semaphore slots*/
			wait_code = WaitForSingleObject(game_semp_handle, WAIT_TIME_DEFAULT/2);
			wait_code2 = WaitForSingleObject(game_semp_handle, WAIT_TIME_DEFAULT/2);
			if (wait_code != WAIT_OBJECT_0 || wait_code2 != WAIT_OBJECT_0) {
				printf("Error when trying to acquire semaphore on service thread\n");
				return ERR_MUTEX;
			}
			was_game = false;
		}
	}
	ret_val = CloseHandle(game_semp_handle);
	if (ret_val == 0) {
		printf("Error releasing mutex\n");
	}
	return (ret_val == 0 ? ERR_MUTEX : 0);			 /* If error releasing mutex return error on exit*/
}

HANDLE start_service_thread() {
	HANDLE ret_handle;
	ret_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServiceThread, NULL, 0, NULL);
	return ret_handle;
}

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
	SOCKET MainSocket = INVALID_SOCKET;
	HANDLE service_thread_handle;
	HANDLE check_exit_handle;
	unsigned long Address;
	SOCKADDR_IN service;
	int ListenRes;
	int bindRes;
	int Loop;
	int Ind;

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
	service_thread_handle = start_service_thread();


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
	//while (!received_exit); --- probelmatic for some reason
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			goto server_cleanup_3;
		}
		printf("Client Connected.\n");
		Ind = FindFirstUnusedThreadSlot(ThreadHandles);

		if (Ind == NUM_OF_WORKER_THREADS) //no slot is available
		{
			printf("No slots available for client, dropping the connection.\n");
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.
		}
		else {
			ThreadInputs[Ind] = AcceptSocket; // shallow copy: don't close, AcceptSocket, instead close, ThreadInputs[Ind] when the time comes.
			ThreadHandles[Ind] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ClientThread, Ind, 0, NULL);
		}
	} // while

	/*need to add code to close thread better and to signal threads for finish.*/

	/*NEED TO ADD CLOSE HANDLE FOR THESE TWO:
	check_exit_handle = start_exit_thread(); 
	service_thread_handle = start_service_thread(); */
server_cleanup_3:
	CleanupWorkerThreads(ThreadHandles, ThreadInputs);

server_cleanup_2:
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());

server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}

int get_response(RX_msg **rx_msg, SOCKET *t_socket) {
	TransferResult_t RecvRes;
	char *AcceptedStr = NULL;

	RecvRes = ReceiveString(&AcceptedStr, *t_socket);

	if (RecvRes == TRNS_FAILED) {
		printf("Service socket error while reading, closing thread.\n");
		closesocket(*t_socket);
		return ERR_SOCKET_TRANS;
	}
	else if (RecvRes == TRNS_DISCONNECTED) {
		printf("Connection closed while reading, closing thread.\n");
		closesocket(*t_socket);
		return ERR_SOCKET_DISCONNECT;
	}
	else {
		printf("Got string : %s\n", AcceptedStr);
		*rx_msg = parse_message_params(AcceptedStr);
		return 0;
	}
	free(AcceptedStr);
}


//Service thread is the thread that opens for each successful client connection and "talks" to the client.
static DWORD ClientThread(int priv_index)
{
	TransferResult_t SendRes = TRNS_SUCCEEDED, SendRes2 = TRNS_SUCCEEDED;
	TransferResult_t SendRes3 = TRNS_SUCCEEDED, SendRes4 = TRNS_SUCCEEDED;
	char username_str[MAX_USERNAME_LEN];
	bool client_chose_versus = true;
	int other_player_status = ERR;
	bool client_chose_cpu = true;
	bool is_second_game = false;
	e_Msg_Type prev_rx_msg;
	RX_msg *rx_msg = NULL;
	bool release_res;
	HANDLE game_semp;
	SOCKET *t_socket;
	BOOL Done = FALSE;
	int err = ERR;

	t_socket = &ThreadInputs[priv_index];
	while (!Done)
	{
		err = get_response(&rx_msg, t_socket);
		if (err) {
			printf("ERROR: Communication with player failed\n");
			err = ERR;
			goto out;
		}
		if (rx_msg->msg_type == CLIENT_REQUEST) {
			prev_rx_msg = CLIENT_REQUEST;
			strcpy(username_str, rx_msg->arg_1);
			printf("I am my name is %s \n", username_str);
			SendRes = send_msg_zero_params(SERVER_APPROVED, *t_socket);
			SendRes2 = send_msg_zero_params(SERVER_MAIN_MENU, *t_socket);
		}
		if (SendRes == TRNS_FAILED || SendRes2 == TRNS_FAILED) {
			printf("Service socket error while writing, closing thread.\n");
			closesocket(*t_socket);
			return ERR_SOCKET_SEND;
		}
		if (rx_msg->msg_type == CLIENT_CPU) {
			client_chose_cpu = true;
			while (client_chose_cpu) {
				client_chose_cpu = false;
				err = start_game_vs_cpu(t_socket, username_str);
				if (err == ERR) {
					printf("Error while playing player vs CPU\n");
					goto out;
				}
				send_msg_zero_params(SERVER_GAME_OVER_MENU, *t_socket);
				err = get_response(&rx_msg, t_socket);
				if (err) {
					printf("Error receiving respons from user\n");
					err = ERR;
				}
				if (rx_msg->msg_type == CLIENT_REPLAY) {
					client_chose_cpu = true;
				}
				else {
					client_chose_cpu = false;
				}
			}
		}
		if (rx_msg->msg_type == CLIENT_VERSUS) {
			client_chose_versus = true;
			other_player_status = wait_for_player_to_join(priv_index, game_status); 	//wait for player to join()
			if (other_player_status == ERR) {
				SendRes3 = send_msg_zero_params(SERVER_NO_OPPONENTS, *t_socket);
				/* Missing send msg SERVER_MAIN_MENU after this msg */
				client_chose_versus = false;
			}

			/*Evertyhing ok: Found opponent, sent msg, now starting VS game*/
			while (client_chose_versus) {   
				client_chose_versus = false;
				err = start_game_vs_player(t_socket, username_str, priv_index);
				game_semp = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, GAME_SEMP_NAME);  /* Open Semaphore handle*/
				release_res = ReleaseSemaphore(game_semp, 1, NULL);			/* Release 1 slot from the semaphore */
				if (err < 0 || release_res == FALSE) {
					printf("Error while playing versus opponent err-%d, rlsrs-%d, lasterror-%d", err,release_res, GetLastError());
					return ERR;
					/* NEED TO REPLACE to: goto out, release handles...;*/
				}
				send_msg_zero_params(SERVER_GAME_OVER_MENU, *t_socket);
				err = get_response(&rx_msg, t_socket);
				game_status[priv_index] = false;		
				if (err) {
					printf("Error receiving response from user\n");
					err = ERR;
				}
				if (rx_msg->msg_type == CLIENT_REPLAY) {
					/* Need from here to go back to the start of if! not WHILE, IF!*/
					is_second_game = true;
					/*Go up, wait for opponent, if opponent not found, search OPPONENT_QUIT and then SERVER_*/
				}
				else {
					client_chose_versus = false;
				}
			}
		}
		if (rx_msg->msg_type == CLIENT_MAIN_MENU) {
			send_msg_zero_params(SERVER_MAIN_MENU, *t_socket);
		}

		/* NOT IN THE RIGHT PLACE */
		if (rx_msg->msg_type == CLIENT_DISCONNECT) {
			err = 0;
			goto out;									/*closing client thread, waiting for new connections.*/
		}

	}
	printf("Conversation ended.\n");
out: 
	closesocket(*t_socket);
	return err;
}
