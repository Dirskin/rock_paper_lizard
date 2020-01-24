#pragma once

#ifndef COMMON_H
#define COMMON_H

/* includes*/


/* defines */
#define NUM_OF_WORKER_THREADS 2
#define MAX_MSG_TYPE_LEN 27 /*SERVER_PLAYER_MOVE_REQUEST+\0*/
#define MAX_USERNAME_LEN 20 /*given in the instructions*/
#define MAX_MOVE_NAME_LEN 9 /*strlen(SCISSORS)*/

/* Wait Periods*/
#define WAIT_TIME_CLIENT_GAME 30000
#define WAIT_TIME_DEFAULT 15000

/* ERR codes*/
#define ERR -1
#define ERR_MALLOC (-2)
	/*socket errors*/
#define ERR_SOCKET (-11)
#define ERR_SOCKET_SEND (-12)
#define ERR_SOCKET_TRANS (-13)
#define ERR_SOCKET_DISCONNECT (-14)
	/*thread sync errors*/
#define ERR_THREAD_WAIT_TIME (-15)
#define ERR_CLOSING_THREAD (-16)
#define ERR_MUTEX (-17)
#define ERR_SEMAPHORE (-18)
#define ERR_WRONG_MSG_RECEIVED (-21)
	/*File errors*/
#define ERR_FILE (-41)
#define TRY_TO_RECONNECT 333
#define EXIT_CONNECTION 334

/* server client defines*/
#define CLIENT_QUITS 100
#define APPROVED_BY_SERVER 110
/*enums*/
typedef enum {
	CLIENT_REQUEST, CLIENT_MAIN_MENU, CLIENT_CPU, CLIENT_VERSUS,
	CLIENT_LEADERBOARD, CLIENT_PLAYER_MOVE, CLIENT_REPLAY,
	CLIENT_REFRESH, CLIENT_DISCONNECT,
	SERVER_MAIN_MENU, SERVER_APPROVED, SERVER_DENIED, SERVER_INVITE,
	SERVER_PLAYER_MOVE_REQUEST, SERVER_GAME_RESULTS, SERVER_GAME_OVER_MENU,
	SERVER_OPPONENT_QUIT, SERVER_NO_OPPONENTS, SERVER_LEADERBOARD,
	SERVER_LEADERBOARD_MENU, ERR_CONNECTION_LOST, ERR_CONNECTION_DENIED
} e_Msg_Type;

typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;

typedef enum  { NOT_DECIDED, WANT_REPLAY, QUIT } player_replay_status;

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

