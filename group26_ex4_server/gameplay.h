#pragma once
#ifndef GAME_PLAY
#define GAME_PLAY

/* includes */
#include <winsock2.h>

/* function declarations*/
int start_game_vs_cpu(SOCKET *t_socket, char *username_str);
int start_game_vs_player(SOCKET *t_socket, char *username_str);
/* gameplay defines */


#endif // GAME_PLAY