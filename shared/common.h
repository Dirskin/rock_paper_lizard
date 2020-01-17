#pragma once

#ifndef COMMON_H
#define COMMON_H

/* includes*/


/* defines */
#define NUM_OF_WORKER_THREADS 2
#define MAX_MSG_TYPE_LEN 27 /*SERVER_PLAYER_MOVE_REQUEST+\0*/
/* ERR codes*/
#define ERR -1
#define ERR_MALLOC (-2)
#define ERR_SOCKET (-11)


/*enums*/
typedef enum {
	CLIENT_REQUEST, CLIENT_MAIN_MENU, CLIENT_CPU, CLIENT_VERSUS,
	CLIENT_LEADERBOARD, CLIENT_PLAYER_MOVE, CLIENT_REPLY,
	CLIENT_REFRESH, CLIENT_DISCONNECT,
	SERVER_MAIN_MENU, SERVER_APPROVED, SERVER_DENIED, SERVER_INVITE,
	SERVER_PLAYER_MOVE_REQUEST, SERVER_GAME_RESULTS, SERVER_GAME_OVER_MENU,
	SERVER_OPPONENT_QUIT, SERVER_NO_OPPONENTS, SERVER_LEADERBOARD,
	SERVER_LEADERBOARD_MENU
} e_Msg_Type;

typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;

/* structs */
typedef struct RX_msg {
	e_Msg_Type msg_type;
	char *arg_1;
	char *arg_2;
	char *arg_3;
	char *arg_4;
} RX_msg;

typedef struct Flow_param {
	char *username;
	char *ip;
	int port;
} Flow_param;


#endif // !COMMON_H

