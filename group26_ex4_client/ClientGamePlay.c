#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include "ClientGamePlay.h"
#include "clientsockethandle.h"
#include "../shared/socket_shared.h"
#include "../shared/SocketSendRecvTools.h"


int play_against_cpu(SOCKET m_socket) {
	char decision[100];
	printf("Choose a move from the list: Rock, Paper, Scissors, Lizard or Spock\n");
	scanf("%s", decision);
	printf("you have choose:%s\n", decision);
	return 0;
}