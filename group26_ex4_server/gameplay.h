#pragma once
#ifndef GAME_PLAY
#define GAME_PLAY

/* includes */
#include <winsock2.h>

/* function declarations*/
int start_game_vs_cpu(SOCKET *t_socket, char *username_str);
int start_game_vs_player(SOCKET *t_socket, char *username_str, int priv_index, char *usernames_str);
int wait_for_player_to_join(int priv_index, bool *game_status);


/* gameplay defines */


#endif // GAME_PLAY