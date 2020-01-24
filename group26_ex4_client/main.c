/* Project 4 - Into to OS */
/* Authors: Ron Diskin, Alex Bogdanov */
/* Description: Main Client module. handles arguments and calling ClientSocketHandle module */


#include <stdio.h>
#include <stdlib.h>
#include "clientsockethandle.h"

int main(int argc, char *argv[])
{
	int err, port = 0;
	char *ip = argv[1], *username = argv[3];
	if (argc < 4) {
		printf("Not enough arguments.\n usage: group26_ex4_client <ip> <port> <username>\n exiting...\n");
		return (-1);
	}
	port = atoi(argv[2]);
	err = MainClient(ip, port, username);
	return err;
}