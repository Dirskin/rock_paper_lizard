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
char *usernames_str[2] = { NULL, NULL };
bool received_exit = false;
bool game_status[2] = { false, false }; /*array to signal if there are two players who wishes to play against each other*/
player_replay_status player_status[2] = { NOT_DECIDED, NOT_DECIDED };


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

bool check_exit_thread_returned(HANDLE check_exit_thread) {
	int ret_val;
	GetExitCodeThread(check_exit_thread, &ret_val);
	if (ret_val != STILL_ACTIVE) {
		if (ret_val == 0) {
			printf("exiting...\n");
			return true;
		}
		else {
			printf("error running check_exit_thread, exiting\n");
			return true;
		}
	}
	else return false;
}

SOCKET nonblock_accept(SOCKET socket) {
	int retval = 0;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(socket, &set);
	struct timeval timeout;
	timeout.tv_usec = 1000;
	timeout.tv_sec = 0;
	retval = select(socket + 1, &set, NULL, NULL, &timeout);
	if (retval == SOCKET_ERROR) {
		printf("Error in select() for non blocking accpet()\n");
		return INVALID_SOCKET;
	}
	if (retval == 1) {
		printf("Server says: new connection detected, accepting\n");
		SOCKET AcceptSocket = accept(socket, NULL, NULL);
		return AcceptSocket;
	}
	return SOCKET_NO_CONNECTIONS;
}

// Initialize all thread handles to NULL, to mark that they have not been initialized
void thread_handle_null_init(HANDLE *ThreadHandles) {
	for (int Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;
}

TransferResult_t drop_third_client(SOCKET AcceptSocket) {
	TransferResult_t retres;

	printf("Server says: 3rd player tries to connect. Server Full. Dropping the connection.\n");
	retres = send_msg_one_param(SERVER_DENIED, AcceptSocket, "Server is full");
	Sleep(20);					/*allow the message to arrive to client before closing the socket to avoid error*/
	closesocket(AcceptSocket); //Closing the socket, dropping the connection.

	return retres;
}

int close_main_socket(MainSocket) {
	if (closesocket(MainSocket) == SOCKET_ERROR) {
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		return SOCKET_ERROR;
	}
	return 0;
}

int MainServer(int port)
{
	HANDLE service_thread_handle, check_exit_handle;
	int ListenRes, bindRes, Ind, err = 0;
	TransferResult_t retres;
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	bool Done = false;

	check_exit_handle = start_exit_thread(); /*Run thread in background to check for "exit" in console*/
	service_thread_handle = start_service_thread();
	if (!check_exit_handle || !service_thread_handle) goto out_handles;

	WSADATA wsaData;	// Initialize Winsock.
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
	Address = inet_addr(SERVER_ADDRESS_STR); 	// create SOCKADDR params and Bind the socket
	if (Address == INADDR_NONE) {
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		goto server_cleanup_2;
	}
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(port); 

	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR) {
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		goto server_cleanup_2;
	}
	ListenRes = listen(MainSocket, SOMAXCONN);	// Listen on the Socket.
	if (ListenRes == SOCKET_ERROR) {
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto server_cleanup_2;
	}
	thread_handle_null_init(ThreadHandles);
	printf("Server Says: Waiting for a clients to connect...\n");
	while(!Done) {
		SOCKET AcceptSocket;
		 do {
			 if (check_exit_thread_returned(check_exit_handle)) {
				 Done = true;
				 break;   /* According to Forum message, "Can assume no exit will
						   * be written to server if clients connected"*/
			 }
			 AcceptSocket = nonblock_accept(MainSocket);
		 } while (AcceptSocket == SOCKET_NO_CONNECTIONS);
		if (Done) break;
		if (AcceptSocket == INVALID_SOCKET) {
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			goto server_cleanup_3;
		}
		printf("Server says: Client Connected.\n");
		Ind = FindFirstUnusedThreadSlot(ThreadHandles);
		if (Ind == NUM_OF_WORKER_THREADS) { //no slot is available
			retres = drop_third_client(AcceptSocket);
			if (retres != TRNS_SUCCEEDED) {
				printf("error sending deny message\n");
				goto server_cleanup_3;
			}
		}
		else {
			ThreadInputs[Ind] = AcceptSocket; // shallow copy: don't close, AcceptSocket, instead close, ThreadInputs[Ind] when the time comes.
			ThreadHandles[Ind] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ClientThread, Ind, 0, NULL);
		}
	} // while
server_cleanup_3:
	CleanupWorkerThreads(ThreadHandles, ThreadInputs);
server_cleanup_2:
	err = close_main_socket(MainSocket) == SOCKET_ERROR ? SOCKET_ERROR : err;
server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR) {
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		err = SOCKET_ERROR;
	}
out_handles:
	if (CloseHandle(check_exit_handle) == false || CloseHandle(service_thread_handle) == false) {
		printf("Error closing thread handles\n");
		err = ERR;
	}
	return err;
}

/* Read, allocate memory, and then return the message structure with parsed arguments*/
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
		*rx_msg = parse_message_params(AcceptedStr);
		return 0;
	}
	free(AcceptedStr);
}


int send_server_invite(int priv_index, SOCKET *t_socket ) {
	TransferResult_t SendRes = TRNS_SUCCEEDED;
	Sleep(10); /*sync with client 2 username_str*/
	SendRes = send_msg_one_param(SERVER_INVITE, *t_socket, usernames_str[!priv_index]);
	if (SendRes == TRNS_FAILED) {
		printf("Service socket error while writing, closing thread.\n");
		closesocket(*t_socket);
		return ERR_SOCKET_SEND;
	}
	return 0;
}

/* waits for opponent to fill his part in player_status[] and decide if wants to replay*/
bool wait_for_opponent_replay_decision(int priv_index) {
	while (player_status[!priv_index] == NOT_DECIDED) {
		continue;
	}
	if (player_status[!priv_index] == WANT_REPLAY) {
		return true;
	}
	else {
		return false;
	}
}

/* Generate first handshake between client to server */
int initiate_client_connection(SOCKET *t_socket, RX_msg *rx_msg, char *username_str) {
	TransferResult_t SendRes = TRNS_SUCCEEDED;
	int err = 0;

	err = get_response(&rx_msg, t_socket);
	if (err < 0) {
		printf("ERROR: Communication with player failed\n");
		return ERR_SOCKET;
	}
	if (rx_msg->msg_type == CLIENT_REQUEST) {
		strcpy(username_str, rx_msg->arg_1);
		printf("Server Says: connected user is '%s' \n", username_str);
		SendRes = send_msg_zero_params(SERVER_APPROVED, *t_socket);
		if (SendRes == TRNS_FAILED) {
			printf("Service socket error while writing, closing thread.\n");
			return ERR_SOCKET;
		}
	}
	return 0;
}

/* check if err value is less than zero, prints message if true*/
bool printf_err(int err, const char* msg) {
	if (err < 0) {
		printf("%s\n", msg);
		return true;
	}
	return false;
}

/* check for error & print if needed. returns true for error. fill error code in err.*/
bool printf_trans_err(TransferResult_t SendRes, const char* msg, int *err) {
	if (SendRes == TRNS_FAILED) {
		printf("Service socket error while writing, closing thread.");
		*err = ERR_SOCKET_SEND;
		return true;
	}
	return false;
}

/*ClientThread is the thread that opens for each successful client connection*/
static DWORD ClientThread(int priv_index)
{
	TransferResult_t SendRes = TRNS_SUCCEEDED, SendRes2 = TRNS_SUCCEEDED;
	TransferResult_t SendRes3 = TRNS_SUCCEEDED, SendRes4 = TRNS_SUCCEEDED;
	bool client_chose_versus = true, client_chose_cpu = true, release_res;
	int other_player_status = ERR, err = ERR;
	char username_str[MAX_USERNAME_LEN];
	HANDLE game_semp = NULL;
	RX_msg *rx_msg = NULL;
	BOOL game_on = TRUE;
	SOCKET *t_socket;

	t_socket = &ThreadInputs[priv_index];
	err = initiate_client_connection(t_socket, rx_msg, username_str);
	if (err < 0) goto out_socket;
	
	while (game_on) {  /* Connection established, Entering Game loop*/
		SendRes2 = send_msg_zero_params(SERVER_MAIN_MENU, *t_socket);
		if(printf_trans_err(SendRes2, "Service socket error while writing, closing thread.", &err)) goto out_socket;

		err = get_response(&rx_msg, t_socket);
		if (printf_err(err, "ERROR: Communication with player failed\n")) goto out_socket;
		/*  -- User menu option 1: play against CPU --*/
		if (rx_msg->msg_type == CLIENT_CPU) {
			client_chose_cpu = true;
			while (client_chose_cpu) {
				client_chose_cpu = false;
				err = start_game_vs_cpu(t_socket, username_str);
				if (printf_err(err, "Error while playing player vs CPU")) goto out_socket;

				send_msg_zero_params(SERVER_GAME_OVER_MENU, *t_socket);
				err = get_response(&rx_msg, t_socket);
				if (printf_err(err, "Error receiving respons from user")) goto out_socket;
				client_chose_cpu = rx_msg->msg_type == CLIENT_REPLAY ? true : false;
			}
		}
		/*  -- User menu option 2: play against other player --*/
		else if (rx_msg->msg_type == CLIENT_VERSUS) {
			client_chose_versus = true;
			other_player_status = wait_for_player_to_join(priv_index, game_status); 	//wait for player to join()
			if (other_player_status == ERR) {
				SendRes3 = send_msg_zero_params(SERVER_NO_OPPONENTS, *t_socket);
				if (printf_trans_err(SendRes3, "Service socket error while writing, closing thread.", &err)) goto out_socket;

				client_chose_versus = false;
			}
			else if (other_player_status == ERR_SEMAPHORE) {
				goto out_socket;
			} else {
				usernames_str[priv_index] = username_str; /*setting current active player username*/
				err = send_server_invite(priv_index, t_socket);
				if (err != 0) goto out_socket;
			}
			/*Evertyhing ok: Found opponent, sent msg, now starting VS game*/
			while (client_chose_versus) {
				player_status[priv_index] = NOT_DECIDED;
				client_chose_versus = false;
				err = start_game_vs_player(t_socket, username_str, priv_index, usernames_str);
				game_semp = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, GAME_SEMP_NAME);  /* Open Semaphore handle*/
				release_res = ReleaseSemaphore(game_semp, 1, NULL);			/* Release 1 slot from the semaphore */
				if (err < 0 || release_res == FALSE) {
					printf("Error while playing versus opponent err-%d, rlsrs-%d, lasterror-%d", err, release_res, GetLastError());
					goto out_semp_socket;
				}
				SendRes2 = send_msg_zero_params(SERVER_GAME_OVER_MENU, *t_socket);
				if (printf_trans_err(SendRes2, "Service socket error while writing, closing thread.", &err)) goto out_socket;
				err = get_response(&rx_msg, t_socket);
				game_status[priv_index] = false;
				if (printf_err(err, "Error receiving response from user")) goto out_semp_socket;
				if (rx_msg->msg_type == CLIENT_REPLAY) {
					player_status[priv_index] = WANT_REPLAY;
					client_chose_versus = wait_for_opponent_replay_decision(priv_index);
					if (!client_chose_versus) { /*Opponent doesnt want to play*/
						SendRes4 = send_msg_zero_params(SERVER_OPPONENT_QUIT, *t_socket);
						printf("Server opponent quit\n");
						if (printf_trans_err(SendRes4, "Service socket error while writing, closing thread.", &err)) goto out_semp_socket;
					}
				}
				else if (rx_msg->msg_type == CLIENT_MAIN_MENU) {
					player_status[priv_index] = QUIT;
					client_chose_versus = false;
				}
				if (CloseHandle(game_semp) == FALSE) {
					printf("error closing semaphore handle\n");
					goto out_socket;
				}
			}
		}
		/*  -- User menu option 3: quit --*/
		else if (rx_msg->msg_type == CLIENT_DISCONNECT) {
			err = 0;
			goto out_socket;		/*closing client thread, waiting for new connections.*/
		}
		free(rx_msg);	/*MALLOC in get_response(), free here */
	}
out_semp_socket:
	if (game_semp) {
		if (CloseHandle(game_semp) == FALSE) {
			printf("error closing semaphore handle, error code %d\n", GetLastError());
		}
	}
out_socket: 
	closesocket(*t_socket);
	return err;
}
