#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> /*for srand()*/
#include "../shared/SocketSendRecvTools.h"
#include "gameplay.h"
#include "serversockethandle.h"
#include "../shared/gameplay_common.h"
#include "../shared/common.h"
#include "../shared/socket_shared.h"

/* winner codes. 5 the is the prefix for this kind of messages*/
#define SERVER_WON 50
#define PLAYER1_WON 51
#define PLAYER2_WON 52
#define TIE 53

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



TransferResult_t send_results_msg(SOCKET *t_socket, int winner, Game_Move cpu_move, char *player_move, char *username_str) {
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

int start_game_vs_cpu(SOCKET *t_socket, char *username_str) {
	TransferResult_t SendRes = TRNS_SUCCEEDED, SendRes2 = TRNS_SUCCEEDED;;
	TransferResult_t RecvRes;
	e_Msg_Type prev_rx_msg;
	Game_Move cpu_move;
	RX_msg *rx_msg = NULL;
	bool Done = false;
	int err, winner = -1;;

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
		RecvRes = send_results_msg(t_socket, winner, cpu_move, rx_msg->arg_1, username_str);
	}
	else {
		winner = winner == 1 ? SERVER_WON : PLAYER1_WON;						 /*playing against CPU, CPU winner code defined 1*/
		RecvRes = send_results_msg(t_socket, winner, cpu_move, rx_msg->arg_1, username_str);
	}
	if (RecvRes != TRNS_SUCCEEDED) {
		return ERR_SOCKET_SEND;
	}
	return 0;
}