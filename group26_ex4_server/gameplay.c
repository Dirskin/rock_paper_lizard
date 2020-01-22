#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> /*for srand()*/
#include "gameplay.h"
#include "serversockethandle.h"
#include "thread_handle.h"
#include "../shared/SocketSendRecvTools.h"
#include "../shared/gameplay_common.h"
#include "../shared/common.h"
#include "../shared/socket_shared.h"

/* winner codes. 5 the is the prefix for this kind of messages*/
#define SERVER_WON 50
#define PLAYER1_WON 51
#define GAME_I_WON 52
#define GAME_OPPONENT_WON 53
#define TIE 53

/* Global file pointer fol all the threads to read*/
FILE *gamesession_file = NULL;
bool wrote_to_file[2] = { false, false };
char *usernames_str[2] = { NULL, NULL };

Game_Move generate_cpu_move() {
	time_t t;
	Game_Move cpu_move;
	srand((unsigned)time(&t));
	cpu_move = rand() % 5;
	return cpu_move;
}

int find_winner(Game_Move player1move, Game_Move player2move) {
	if (player1move == player2move) { /* tie */
		return TIE;
	}
	/* Player 1 possible wins*/
	if (player1move == ROCK && player2move == SCISSORS || player1move == ROCK && player2move == LIZARD) {
		return 1;
	}
	if (player1move == PAPER && player2move == ROCK || player1move == PAPER && player2move == SPOCK) {
		return 1;
	}
	if (player1move == SCISSORS && player2move == PAPER || player1move == SCISSORS && player2move == LIZARD) {
		return 1;
	}
	if (player1move == LIZARD && player2move == PAPER || player1move == LIZARD && player2move == SPOCK) {
		return 1;
	}
	if (player1move == SPOCK && player2move == ROCK || player1move == SPOCK && player2move == SCISSORS) {
		return 1;
	}
	/* Player 2 possible wins:*/
	if (player2move == ROCK && player1move == SCISSORS || player2move == ROCK && player1move == LIZARD) {
		return 2;
	}
	if (player2move == PAPER && player1move == ROCK || player2move == PAPER && player1move == SPOCK) {
		return 2;
	}
	if (player2move == SCISSORS && player1move == PAPER || player2move == SCISSORS && player1move == LIZARD) {
		return 2;
	}
	if (player2move == LIZARD && player1move == PAPER || player2move == LIZARD && player1move == SPOCK) {
		return 2;
	}
	if (player2move == SPOCK && player1move == ROCK || player2move == SPOCK && player1move == SCISSORS) {
		return 2;
	}
	return ERR;
}

TransferResult_t send_results_msg_cpu(SOCKET *t_socket, int winner, Game_Move cpu_move, char *player_move, char *username_str) {
	char server_name[MAX_USERNAME_LEN];
	char cpu_move_str[MAX_MOVE_NAME_LEN];
	TransferResult_t err;

	strcpy(server_name, "Server");

	/*enum to str:*/
	switch (cpu_move) {
	case ROCK:
		strcpy(cpu_move_str, "ROCK");
		break;
	case PAPER:
		strcpy(cpu_move_str, "PAPER");
		break;
	case SCISSORS:
		strcpy(cpu_move_str, "SCISSORS");
		break;
	case LIZARD:
		strcpy(cpu_move_str, "LIZARD");
		break;
	case SPOCK:
		strcpy(cpu_move_str, "SPOCK");
		break;
	}
	/* Sending By correct argument order*/
	if (winner == SERVER_WON) {
		err = send_msg_quad_params(SERVER_GAME_RESULTS, *t_socket, server_name, cpu_move_str, player_move, server_name);
	}
	else if(winner == PLAYER1_WON) {
		err = send_msg_quad_params(SERVER_GAME_RESULTS, *t_socket, server_name, cpu_move_str, player_move, username_str);
	}
	else if (winner == TIE) {
		err = send_msg_quad_params(SERVER_GAME_RESULTS, *t_socket, server_name, cpu_move_str, player_move, "TIE");
	}
	return err;
}


TransferResult_t send_results_msg_human(SOCKET *t_socket, int winner, Game_Move opponent_move, char *player_move, int priv_index) {
	char *opponent_name = usernames_str[!priv_index];
	char *my_name = usernames_str[priv_index];
	char opponent_move_str[MAX_MOVE_NAME_LEN];
	TransferResult_t err;

	/*enum to str:*/
	switch (opponent_move) {
	case ROCK:
		strcpy(opponent_move_str, "ROCK");
		break;
	case PAPER:
		strcpy(opponent_move_str, "PAPER");
		break;
	case SCISSORS:
		strcpy(opponent_move_str, "SCISSORS");
		break;
	case LIZARD:
		strcpy(opponent_move_str, "LIZARD");
		break;
	case SPOCK:
		strcpy(opponent_move_str, "SPOCK");
		break;
	}
	/* Sending By correct argument order*/
	if (winner == GAME_I_WON) {
		err = send_msg_quad_params(SERVER_GAME_RESULTS, *t_socket, opponent_name, opponent_move_str, player_move, my_name);
	}
	else if (winner == GAME_OPPONENT_WON) {
		err = send_msg_quad_params(SERVER_GAME_RESULTS, *t_socket, opponent_name, opponent_move_str, player_move, opponent_name);
	}
	else if (winner == TIE) {
		err = send_msg_quad_params(SERVER_GAME_RESULTS, *t_socket, opponent_name, opponent_move_str, player_move, "TIE");
	}
	return err;
}



int start_game_vs_cpu(SOCKET *t_socket, char *username_str) {
	TransferResult_t SendRes = TRNS_SUCCEEDED, SendRes2 = TRNS_SUCCEEDED;;
	TransferResult_t RecvRes;
	RX_msg *rx_msg = NULL;
	int err, winner = -1;
	Game_Move cpu_move;
	bool Done = false;

	SendRes = send_msg_zero_params(SERVER_PLAYER_MOVE_REQUEST, *t_socket);
	if (SendRes == TRNS_FAILED) return ERR_SOCKET_SEND;
	err = get_response(&rx_msg, t_socket);
	if (rx_msg->msg_type != CLIENT_PLAYER_MOVE) return ERR_WRONG_MSG_RECEIVED;
	if (err < 0) {
		printf("Error receiving message while playing CPU vs player\n");
		return ERR;
	}
	cpu_move = generate_cpu_move();
	winner = find_winner(cpu_move, identify_game_move(rx_msg->arg_1));			/* converting move(string) to GameMove eNum, then comparing */
	if (winner == TIE) {
		RecvRes = send_results_msg_cpu(t_socket, winner, cpu_move, rx_msg->arg_1, username_str);
	}
	else {
		winner = winner == 1 ? SERVER_WON : PLAYER1_WON;						 /*playing against CPU, CPU winner code defined 1*/
		RecvRes = send_results_msg_cpu(t_socket, winner, cpu_move, rx_msg->arg_1, username_str);
	}
	if (RecvRes != TRNS_SUCCEEDED) {
		return ERR_SOCKET_SEND;
	}
	return 0;
}

bool file_exists()
{
	if ((gamesession_file = fopen("GameSession.txt", "r")))
	{
		fclose(gamesession_file);
		return true;
	}
	return false;
}
/*reads one line from file, returns Game_Move type*/
Game_Move read_file_and_return_move() {
	Game_Move opponent_move;
	fscanf(gamesession_file, "%d", &opponent_move);
	return opponent_move;
}

void write_my_move_to_file(Game_Move my_move, int priv_index) {
	fseek(gamesession_file, 0, SEEK_SET); /*rewinds file location so the other player will read what we wrote*/
	printf("ftell is: %d", ftell(gamesession_file));
	fprintf(gamesession_file, "%d\n", my_move);
	wrote_to_file[priv_index] = true;
}

int open_file_and_write_move(Game_Move my_move, int priv_index) {
	gamesession_file = fopen("GameSession.txt", "a+");
	if (!gamesession_file) {
		printf("File open error \n");
		return ERR_FILE;
	}
	write_my_move_to_file(my_move, priv_index);
	return 0;
}

Game_Move read_opponent_move_append_mine(int priv_index, char* arg_1, bool write_mine) {
	Game_Move opponent_move, my_move;
	gamesession_file = fopen("GameSession.txt", "r+");
	if (!gamesession_file) {
		printf("Error trying to read GameSession.txt file\n");
		return ERR_FILE;
	}
	fseek(gamesession_file, 0, SEEK_SET);
	opponent_move = read_file_and_return_move();
	if (write_mine) {
		my_move = identify_game_move(arg_1);
		write_my_move_to_file(my_move, priv_index);
	}
	fclose(gamesession_file);
	return opponent_move;
}

int start_game_vs_player(SOCKET *t_socket, char *username_str, int priv_index) {
	TransferResult_t SendRes = TRNS_SUCCEEDED, SendRes2 = TRNS_SUCCEEDED, SendRes3 = TRNS_SUCCEEDED;
	HANDLE file_mutex_handle = NULL;
	TransferResult_t RecvRes;
	RX_msg *rx_msg = NULL;
	int err, winner = -1;
	Game_Move opponent_move, my_move;
	bool Done = false;
	bool ret_val;
	DWORD wait_code;

	wrote_to_file[priv_index] = false;
	usernames_str[priv_index] = username_str; /*setting current active player username*/
	Sleep(10);
	SendRes3 = send_msg_one_param(SERVER_INVITE, *t_socket, usernames_str[!priv_index]);
	if (SendRes3 == TRNS_FAILED) {
		printf("Service socket error while writing, closing thread.\n");
		closesocket(*t_socket);
		return ERR_SOCKET_SEND;
	}

	file_mutex_handle = OpenMutex(SYNCHRONIZE, FALSE, FILE_MUTEX_NAME); /*Open named mutex*/
	wait_code = WaitForSingleObject(file_mutex_handle, INFINITE); /*Waiting for other player decision - INFINITE wait according to instructions*/
	if (wait_code != WAIT_OBJECT_0) {
		printf("Error when waiting for file mutex\n");
		return ERR_MUTEX;
	}
	/*Acquired mutex, now in protected zone, can access file*/
	/*Asking Client Move*/
	SendRes = send_msg_zero_params(SERVER_PLAYER_MOVE_REQUEST, *t_socket);
	if (SendRes == TRNS_FAILED) return ERR_SOCKET_SEND;
	err = get_response(&rx_msg, t_socket);
	if (rx_msg->msg_type != CLIENT_PLAYER_MOVE) return ERR_WRONG_MSG_RECEIVED;
	if (err < 0) {
		printf("Error receiving message while playing vs player\n");
		return ERR;
	}

	/* Check if need to write + read, or only read from file*/
	my_move = identify_game_move(rx_msg->arg_1);
	if (file_exists()) {
		opponent_move = read_opponent_move_append_mine(priv_index, rx_msg->arg_1, true);
		if (opponent_move < 0) return ERR_FILE;
	}
	else if (!file_exists()) {
		open_file_and_write_move(my_move, priv_index);
		fclose(gamesession_file);
		ret_val = ReleaseMutex(file_mutex_handle);
		if (ret_val == FALSE) {
			printf("Error when releasing file mutex\n");
			return ERR_MUTEX;
		}
		while (!wrote_to_file[!priv_index]) { /*waiting for other opponent to write his move*/
			;/*waiting....*/
		}
		opponent_move = read_opponent_move_append_mine(priv_index, rx_msg->arg_1, false);
		if (opponent_move < 0) return ERR_FILE;
		wait_code = WaitForSingleObject(file_mutex_handle, INFINITE); /*Acquiring file mutex again - read opponent move*/
		if (wait_code != WAIT_OBJECT_0) {
			printf("Error when waiting for file mutex\n");
			return ERR_MUTEX;
		}
	}
	ret_val = ReleaseMutex(file_mutex_handle);
	if (ret_val == FALSE) {
		printf("Error when releasing file mutex\n");
		return ERR_MUTEX;
	}
	winner = find_winner(opponent_move, my_move);			/* converting move(string) to GameMove eNum, then comparing */
	if (winner == TIE) {
		RecvRes = send_results_msg_human(t_socket, winner, opponent_move, rx_msg->arg_1, priv_index);
	}
	else {
		winner = winner == 1 ? GAME_OPPONENT_WON : GAME_I_WON;						 /*playing against CPU, CPU winner code defined 1*/
		RecvRes = send_results_msg_human(t_socket, winner, opponent_move, rx_msg->arg_1, priv_index);
	}
	if (RecvRes != TRNS_SUCCEEDED) {
		return ERR_SOCKET_SEND;
	}
	return 0;
}


int wait_for_player_to_join(int priv_index, bool *game_status) {
	HANDLE game_semp;
	DWORD wait_code;

	game_status[priv_index] = true;									/* Marks myself as waiting for opponent*/
	game_semp = OpenSemaphore(SYNCHRONIZE, FALSE, GAME_SEMP_NAME);  /* Open Semaphore handle*/
	wait_code = WaitForSingleObject(game_semp, WAIT_TIME_CLIENT_GAME);
	if (wait_code != WAIT_OBJECT_0) {
		printf("Error when waiting for opponent - No opponent found\n");
		CloseHandle(game_semp);
		game_status[priv_index] = false;
		return ERR;						/* No Opponent found, send NO_OPPONENT msg*/
	}
	/*opponent found, can start playing*/


	//NEED TO MOVE THIS TO THE CORRECT PLACE AFTER FINISHING!
	return 0;
}