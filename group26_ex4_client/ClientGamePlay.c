#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <string.h>
#include <winsock2.h>
#include "ClientGamePlay.h"
#include "clientsockethandle.h"
#include "../shared/socket_shared.h"
#include "../shared/SocketSendRecvTools.h"
#include "../shared/gameplay_common.h"
#include "../shared/common.h"


void strupper(char *str,int len) {
	for (int i = 0; i < len; i++) {
		if (str[i] >= 'a' && str[i] <= 'z')
		{
			str[i] = str[i] - 32;
		}

	}
}

//printing main menu to the client
int ClientMainMenu(SOCKET m_socket) {
	TransferResult_t SendRes = TRNS_SUCCEEDED;
	char decision[MAX_MSG_TYPE_LEN];
	bool not_valid_input = TRUE;
	int client_move = 0;
	int msg_sent = 0;



	printf("Choose what to do next:\n");
	printf("1. Play against another client\n");
	printf("2. Play against the server\n");
	printf("3. Quit\n");
	scanf("%s", decision);
	while (not_valid_input) {
		client_move = atoi(decision);
		switch (client_move) {
		case 1:
			SendRes = send_msg_zero_params(CLIENT_VERSUS, m_socket);
			msg_sent = CLIENT_VERSUS;
			not_valid_input = FALSE;
			break;
		case 2:
			SendRes = send_msg_zero_params(CLIENT_CPU, m_socket);
			msg_sent = CLIENT_CPU;
			not_valid_input = FALSE;
			break;
		case 3:
			SendRes = send_msg_zero_params(CLIENT_DISCONNECT, m_socket);
			msg_sent = CLIENT_DISCONNECT;
			not_valid_input = FALSE;
			break;
		default:
			printf("please enter valid answer:");
			scanf("%s", decision);
		}
	}
	if (SendRes == TRNS_FAILED) {
			return EXIT_CONNECTION;
		}
	return msg_sent;
}


int play_against_cpu(SOCKET m_socket) {
	char decision[MAX_MOVE_NAME_LEN+10];
	Game_Move move = ERR;
	TransferResult_t SendRes = TRNS_SUCCEEDED;
	printf("Choose a move from the list: Rock, Paper, Scissors, Lizard or Spock\n");

	while (move == ERR) {
		scanf("%s", decision);
		strupper(decision, strlen(decision));
		printf("your decision is %s\n", decision);
		move = identify_game_move(decision);
		if (move == ERR) {
			printf("please enter a valid move:\n");
		}
	}
	switch (move) {
	case ROCK:
		SendRes = send_msg_one_param(CLIENT_PLAYER_MOVE ,m_socket, "ROCK");
		break;
	case PAPER:
		SendRes = send_msg_one_param(CLIENT_PLAYER_MOVE ,m_socket, "PAPER");
		break;
	case SCISSORS:
		SendRes = send_msg_one_param(CLIENT_PLAYER_MOVE ,m_socket, "SCISSORS");
		break;
	case LIZARD :
		SendRes = send_msg_one_param(CLIENT_PLAYER_MOVE ,m_socket, "LIZARD");
		break;
	case SPOCK:
		SendRes = send_msg_one_param(CLIENT_PLAYER_MOVE ,m_socket, "SPOCK");
		break;
	}
	if (SendRes == TRNS_FAILED) {
		return EXIT_CONNECTION;
	}
	return move;
}

int game_play_results(SOCKET m_socket, RX_msg *rx_msg, char *username) {
	char player_move[MAX_MOVE_NAME_LEN], opponent_move[MAX_MOVE_NAME_LEN];
	char winner_name[MAX_USERNAME_LEN], opponent_name[MAX_USERNAME_LEN];

	strcpy(opponent_name, rx_msg->arg_1);
	strcpy(opponent_move, rx_msg->arg_2);
	strcpy(player_move  , rx_msg->arg_3);
	strcpy(winner_name  , rx_msg->arg_4);

	printf("You played: %s\n", player_move);
	printf("%s played:%s\n", opponent_name, opponent_move);
	if (strcmp(winner_name, "TIE")) {
		if (strcmp(winner_name, opponent_name)) {
			printf("%s won!\n", username);
		}
		else {
			printf("%s won!\n", opponent_name);
		}
	}

	return 0;
}

int ClientGameOverMenu(SOCKET m_socket) {
	int game_over_decision;
	int not_valid_input = 1;
	int game_over_play;
	TransferResult_t SendRes = TRNS_SUCCEEDED;
	printf("Choose what to do next:\n");
	printf("1. Play again\n");
	printf("2. Return to the main menu\n");
	scanf("%d", &game_over_decision);
	while (not_valid_input) {
		if (game_over_decision == 1) {
			SendRes = send_msg_zero_params(CLIENT_REPLAY, m_socket);
			not_valid_input = 0;
			game_over_play = CLIENT_REPLAY;
		}

		else if (game_over_decision == 2) {
			SendRes = send_msg_zero_params(CLIENT_MAIN_MENU, m_socket);
			not_valid_input = 0;
			game_over_play = CLIENT_MAIN_MENU;
		}
		else {
			printf("enter a valid input:\n");
			scanf("%d", &game_over_decision);
		}
	}
	if (SendRes == TRNS_FAILED) {
		return EXIT_CONNECTION;
	}
	return game_over_play;

}