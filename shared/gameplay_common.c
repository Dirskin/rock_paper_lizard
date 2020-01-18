#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdbool.h> 
#include <stdio.h>
#include <string.h>

#include "../shared/gameplay_common.h"
#include "../shared/common.h"
#include "../shared/socket_shared.h"   /*for string_are_equals*/
#define MAX_MSG_LEN_ZERO_PARAMS 29 //"SERVER_PLAYER_MOVE_REQUEST"(26) +":"(1) +"\n" +"\0"  

Game_Move identify_game_move(char *game_move) {
	if (STRINGS_ARE_EQUAL(game_move, "ROCK")) {
		return ROCK;
	}
	if (STRINGS_ARE_EQUAL(game_move, "PAPER")) {
		return PAPER;
	}
	if (STRINGS_ARE_EQUAL(game_move, "SCISSORS")) {
		return SCISSORS;
	}
	if (STRINGS_ARE_EQUAL(game_move, "LIZARD")) {
		return LIZARD;
	}
	if (STRINGS_ARE_EQUAL(game_move, "SPOCK")) {
		return SPOCK;
	}
	return ERR;
}