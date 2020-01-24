/* Project 4 - Intro to OS */
/* Authors: Ron Diskin, Alex Bogdanov */
/* Description: Main Server module. handles arguments and calling ServerSocketHandle module */

#include <stdio.h>
#include <stdlib.h>
#include "serversockethandle.h"


int main(int argc, char *argv[])
{
	int port = 0;
	if (argc < 2) {
		printf("Missing input port. usage: group26_ex4_server <port>\n exiting...\n");
		return (-1);
	}
	port = atoi(argv[1]);
	printf("listening on %d...\n", port);
	return MainServer(port);
}