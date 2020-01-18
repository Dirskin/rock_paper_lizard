#pragma once

#ifndef GAMEPLAY_COMMON
#define GAMEPLAY_COMMON

/* includes*/


/* defines */

/* ERR codes*/



/*enums*/
typedef enum {
	ROCK, PAPER, SCISSORS, LIZARD, SPOCK
} Game_Move;


/*functions*/
Game_Move identify_game_move(char *game_move);

#endif // !GAMEPLAY_COMMON

