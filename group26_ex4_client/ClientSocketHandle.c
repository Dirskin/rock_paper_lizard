/*Main Client module, this modules activates the client threads - reading and writing to the server*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include "clientsockethandle.h"
#include "ClientGamePlay.h"
#include "../shared/socket_shared.h"
#include "../shared/SocketSendRecvTools.h"
#include "../shared/common.h"


SOCKET m_socket;
expecting_user_input=false;
bool start_connection = true;
bool threads_are_alive = TRUE;
SOCKADDR_IN clientService;

/*main failure prints*/
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
		scanf("%d", &decision);
		if (decision == 1) { //chooses to try to reconnect
			printf("thanks for choosing to reconnect\n");
			return TRY_TO_RECONNECT;
		}
		else if (decision == 2) { //chooses to exit
			printf("thanks for choosing to exit, bye bye\n");
			return EXIT_CONNECTION;
		}
		else {
			printf("please enter a valid answer\n");
		}

	}
	return ERR;
}

/*Reading from the server thread, reads the server that is sent by the server*/
static DWORD RecvDataThread(RX_msg *rx_msg)
{
	TransferResult_t RecvRes = -1;
	char *AcceptedStr = NULL;
	int wait_time = 15; 

	RecvRes = ReceiveString(&AcceptedStr, m_socket);

	if ((RecvRes == TRNS_FAILED || RecvRes == TRNS_DISCONNECTED)) {
		rx_msg = (RX_msg*)malloc(sizeof(RX_msg));
		if (!rx_msg) {
			free(rx_msg);
		}
		rx_msg->msg_type = ERR_CONNECTION_LOST;
		return (DWORD)rx_msg;
	}

	else {
		rx_msg = parse_message_params(AcceptedStr);
		return (DWORD)rx_msg;
		}

	return (DWORD)0;
}

/*creating new connection, sends CLIENT REQUEST and waiting for server approved*/
int CreateNewConnectionServer(Flow_param *flow_param) {
	TransferResult_t SendRes;
	HANDLE hThread;
	DWORD wait_code;
	BOOL ret_val;
	RX_msg *rx_msg;
	int client_move;
	char SendStr[256];
	bool connecting = true;
	int socket_err;
	bool trying_to_connect = false;

	while (connecting) {
		connecting = false;
		strcpy(SendStr, flow_param->username);
		SendRes = send_msg_one_param(CLIENT_REQUEST, m_socket, flow_param->username);
		if (SendRes == TRNS_FAILED) {
			client_move = failed_connection(flow_param->ip, flow_param->port, F_SERVER_CONNECTION_FAILED);
			if (client_move == TRY_TO_RECONNECT) {
				connecting = true;
				continue;
			}
			else {
				return EXIT_CONNECTION;
			}
		}
		/*passing the rx_msg in order to know which type of message we recieve from the server*/
		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvDataThread, &rx_msg, 0, NULL);

		/*waiting for 15 seconds for response, if the wait time is bigger then assuming the connection is lost*/
		wait_code = WaitForSingleObject(hThread, WAIT_TIME_DEFAULT); //RESPONSE time is set to infinite sec NEED TO BE 15!
		if (WAIT_OBJECT_0 != wait_code)
		{
			printf("Waited for 15 seconds, server lost\n");
			TerminateThread(hThread, 0x555);
			return ERR_THREAD_WAIT_TIME;
		}
		GetExitCodeThread(hThread, &rx_msg);
		ret_val = CloseHandle(hThread);
		if (FALSE == ret_val)
		{
			printf("Error when closing thread: %d\n", GetLastError());
			return ERR_CLOSING_THREAD;                        //NEED TO BE CHANGED TO DEFINE
		}
		if (rx_msg->msg_type == ERR_CONNECTION_LOST) {
			client_move = failed_connection(flow_param->ip, flow_param->port, F_SERVER_CONNECTION_LOST);
			if (client_move == TRY_TO_RECONNECT) {
				connecting = true;
				continue;
			}
			else {
				return EXIT_CONNECTION;
			}
		}
		/*--- third client is trying to connect to the server, denying his connection ---*/
		else if (rx_msg->msg_type == SERVER_DENIED) {
			closesocket(m_socket);
			client_move = failed_connection(flow_param->ip, flow_param->port, F_SERVER_DENIED_CONNECTION);
			if (client_move == TRY_TO_RECONNECT) {
				connecting = true;
				socket_err = connet_to_socket(clientService, flow_param->ip, flow_param->port);
				if (socket_err == EXIT_CONNECTION) {
					free(rx_msg);
					return EXIT_CONNECTION;
				}
				else{
					continue; }
			}
			else {
				free(rx_msg);
				return EXIT_CONNECTION;
			}
		}
		//recieved server connected message 
		else if (rx_msg->msg_type == SERVER_APPROVED) {
			return APPROVED_BY_SERVER;
		}

	}
	return 0;
}

/*Sending data thread - main client thread*/
static DWORD SendDataThread(Flow_param *flow_param)
{
	TransferResult_t SendRes;
	HANDLE hThread;
	DWORD wait_code;
	BOOL ret_val;
	e_Msg_Type msg_rcv = 0;
	int client_menu_select = 0;
	int failed_menu_select = 0;
	RX_msg *rx_msg = NULL;
	char opponent_name[MAX_USERNAME_LEN];
	int socket_err;
	bool trying_to_connect = false;

	while (threads_are_alive){
		/*Starting a new connection to the server, the only valid message is server_connected*/
		if (start_connection){
			start_connection = false;
			client_menu_select = CreateNewConnectionServer(flow_param);
			if (client_menu_select == EXIT_CONNECTION) {
				return EXIT_CONNECTION;
			}
			if (client_menu_select == TRY_TO_RECONNECT){
				start_connection = true;
			}
			continue;  }
		/*Reading from server thread*/
		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvDataThread, &rx_msg, 0, NULL);
		wait_code = WaitForSingleObject(hThread, INFINITE); //RESPONSE time is set to infinite sec NEED TO BE 15!
		if (WAIT_OBJECT_0 != wait_code) {
			printf("Waited for 15 seconds, server lost\n");
			TerminateThread(hThread, 0x555);
		}
		/*Getting the command from the thread*/
		GetExitCodeThread(hThread, &rx_msg);
		ret_val = CloseHandle(hThread);
		if (FALSE == ret_val) {
			printf("Error when closing thread: %d\n", GetLastError());
			free(rx_msg);
			return ERR; }
		switch (rx_msg->msg_type) {
		/*Connection from the server is gone*/
		case (ERR_CONNECTION_LOST):
			client_menu_select = failed_connection(flow_param->ip, flow_param->port, F_SERVER_CONNECTION_LOST);
		case (SERVER_MAIN_MENU):			
			client_menu_select = ClientMainMenu(m_socket);
			if (client_menu_select == CLIENT_DISCONNECT) {
				threads_are_alive = FALSE;
				break; }
			else 
				break;						
		case (SERVER_INVITE):
			printf("Found opponent, game will start soon\n");
			threads_are_alive = TRUE;
			break;
		case (SERVER_PLAYER_MOVE_REQUEST) :
			client_menu_select = play_against_cpu(m_socket);
			break;
		case (SERVER_GAME_RESULTS) :
			msg_rcv = game_play_results(m_socket, rx_msg, flow_param->username, opponent_name);
			break;
		case(SERVER_GAME_OVER_MENU):
			client_menu_select = ClientGameOverMenu(m_socket);
			break;
		case (SERVER_NO_OPPONENTS):
			printf("Couldn't find opponent\n");
			threads_are_alive = TRUE;
			break;
		case(SERVER_OPPONENT_QUIT):
			printf("%s has left the game!\n", opponent_name);
			threads_are_alive = TRUE;
			break;
		}
		if (client_menu_select == EXIT_CONNECTION) {
			failed_menu_select = failed_connection(flow_param->ip, flow_param->port, F_SERVER_CONNECTION_LOST);
			if (failed_menu_select == TRY_TO_RECONNECT) {
				start_connection = true;
				socket_err = connet_to_socket(clientService, flow_param->ip, flow_param->port);
				if (socket_err == EXIT_CONNECTION) {
					free(rx_msg);
					return EXIT_CONNECTION;
				}
				else {
					continue;
				}
			}
			else if (failed_menu_select == EXIT_CONNECTION || client_menu_select == CLIENT_DISCONNECT) {
				free(rx_msg);
				return EXIT_CONNECTION;
			}
		}
	}
	free(rx_msg);
	return (DWORD)0;
}

/*connecting to the socket*/
int connet_to_socket(SOCKADDR_IN clientService, const char *server_ip_addr, int server_port) {
	int socket_err;
	int menu_choosing;
	bool trying_to_connect = true;
	while (trying_to_connect) {
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
		trying_to_connect = false;
		socket_err = connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService));
		if (socket_err == SOCKET_ERROR) {
			menu_choosing = failed_connection(server_ip_addr, server_port, F_SERVER_CONNECTION_FAILED);
			if (menu_choosing == TRY_TO_RECONNECT) {
				closesocket(m_socket);
				trying_to_connect = true;
				continue;
			}
			else if (socket_err == EXIT_CONNECTION);
			return EXIT_CONNECTION;
		}
		else {
			trying_to_connect = false;
			return 0;
		}
	}

}

/*Main client function, creats the sending data thread*/
int MainClient(const char *server_ip_addr, int server_port, char *username) {
	Flow_param flow_param;
	DWORD wait_code;
	BOOL ret_val;
	HANDLE hThread;
	int err;
	bool trying_to_connect = true;
	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()\n");

	//Initializing the struct 
	flow_param.ip = server_ip_addr;
	flow_param.port = server_port;
	flow_param.username = username;
	printf("the username is:%s\n", flow_param.username);
	err = connet_to_socket(clientService, server_ip_addr, server_port);
	if (err == EXIT_CONNECTION) {
		goto out;/*Thats not an error: user chose to exit*/
	}				
	printf("Connected to server on %s:%d\n", server_ip_addr, server_port);
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendDataThread, &flow_param, 0, NULL);
	wait_code = WaitForSingleObject(hThread, INFINITE);
	GetExitCodeThread(hThread, &err);
	ret_val = CloseHandle(hThread);
	if (FALSE == ret_val) {
		printf("Error when closing thread: %d\n", GetLastError());
		return ERR;
	}
out:
	/*no threads are open here*/
	closesocket(m_socket);
	WSACleanup();
	return 0;
}
