#include <stdio.h>
#include <stdlib.h>
#include "clientsockethandle.h"

int main(int argc, char *argv[])
{
	int err, port = 0;
	if (argc < 3) {
		printf("Not enough arguments.\n usage: group26_ex4_client <ip> <port>\n exiting...\n");
		return (-1);
	}
	port = atoi(argv[2]);
	err = MainClient(argv[1], port);
	return err;
}