#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdbool.h> 
#include <stdio.h>
#include <string.h>

#include "SocketSendRecvTools.h"
#define MAX_MSG_LEN_ZERO_PARAMS 29 //"SERVER_PLAYER_MOVE_REQUEST"(26) +":"(1) +"\n" +"\0"  

/*this function sends messages with zero params only*/
bool send_msg_zero_params(e_Msg_Type msg_type, SOCKET t_socket) {
	char sendbuf[MAX_MSG_LEN_ZERO_PARAMS];
	TransferResult_t res;
	switch (msg_type) {
	case SERVER_MAIN_MENU:
		strcpy(sendbuf, "SERVER_MAIN_MENU:\n");
		break;
	case SERVER_APPROVED:
		strcpy(sendbuf, "SERVER_APPROVED:\n");
		break;
	case SERVER_PLAYER_MOVE_REQUEST:
		strcpy(sendbuf, "SERVER_PLAYER_MOVE_REQUEST:\n");
		break;
	case SERVER_NO_OPPONENTS:
		strcpy(sendbuf, "SERVER_PLAYER_MOVE_REQUEST:\n");
		break;
	case SERVER_LEADERBOARD_MENU:
		strcpy(sendbuf, "SERVER_LEADERBOARD_MENU:\n");
		break;
	case CLIENT_MAIN_MENU:
		strcpy(sendbuf, "CLIENT_MAIN_MENU:\n");
		break;
	case CLIENT_CPU:
		strcpy(sendbuf, "CLIENT_CPU:\n");
		break;
	case CLIENT_VERSUS:
		strcpy(sendbuf, "CLIENT_VERSUS:\n");
		break;
	case CLIENT_LEADERBOARD:
		strcpy(sendbuf, "CLIENT_LEADERBOARD:\n");
		break;
	case CLIENT_REPLY:
		strcpy(sendbuf, "CLIENT_REPLY:\n");
		break;
	case CLIENT_REFRESH:
		strcpy(sendbuf, "CLIENT_REFRESH:\n");
		break;
	case CLIENT_DISCONNECT:
		strcpy(sendbuf, "CLIENT_DISCONNECT:\n");
		break;
	}
	res = SendString(sendbuf, t_socket);
	return (res == TRNS_SUCCEEDED ? true : false);
}

TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd)
{
	const char* CurPlacePtr = Buffer;
	int BytesTransferred;
	int RemainingBytesToSend = BytesToSend;

	while (RemainingBytesToSend > 0)
	{
		/* send does not guarantee that the entire message is sent */
		BytesTransferred = send(sd, CurPlacePtr, RemainingBytesToSend, 0);
		if (BytesTransferred == SOCKET_ERROR)
		{
			printf("send() failed, error %d\n", WSAGetLastError());
			return TRNS_FAILED;
		}

		RemainingBytesToSend -= BytesTransferred;
		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

TransferResult_t SendString(const char *Str, SOCKET sd)
{
	int TotalStringSizeInBytes;
	TransferResult_t SendRes;

	/* The request is sent in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	TotalStringSizeInBytes = (int)(strlen(Str) + 1); // terminating zero also sent	

	SendRes = SendBuffer(
		(const char *)(&TotalStringSizeInBytes),
		(int)(sizeof(TotalStringSizeInBytes)), // sizeof(int) 
		sd);

	if (SendRes != TRNS_SUCCEEDED) return SendRes;

	SendRes = SendBuffer(
		(const char *)(Str),
		(int)(TotalStringSizeInBytes),
		sd);

	return SendRes;
}

TransferResult_t ReceiveBuffer(char* OutputBuffer, int BytesToReceive, SOCKET sd)
{
	char* CurPlacePtr = OutputBuffer;
	int BytesJustTransferred;
	int RemainingBytesToReceive = BytesToReceive;

	while (RemainingBytesToReceive > 0)
	{
		/* send does not guarantee that the entire message is sent */
		BytesJustTransferred = recv(sd, CurPlacePtr, RemainingBytesToReceive, 0);
		if (BytesJustTransferred == SOCKET_ERROR)
		{
			printf("recv() failed, error %d\n", WSAGetLastError());
			return TRNS_FAILED;
		}
		else if (BytesJustTransferred == 0)
			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.

		RemainingBytesToReceive -= BytesJustTransferred;
		CurPlacePtr += BytesJustTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd)
{
	int TotalStringSizeInBytes;
	TransferResult_t RecvRes;
	char* StrBuffer = NULL;

	if ((OutputStrPtr == NULL) || (*OutputStrPtr != NULL))
	{
		printf("The first input to ReceiveString() must be "
			"a pointer to a char pointer that is initialized to NULL. For example:\n"
			"\tchar* Buffer = NULL;\n"
			"\tReceiveString( &Buffer, ___ )\n");
		return TRNS_FAILED;
	}

	/* The request is received in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */
	RecvRes = ReceiveBuffer(
		(char *)(&TotalStringSizeInBytes),
		(int)(sizeof(TotalStringSizeInBytes)), // 4 bytes
		sd);

	if (RecvRes != TRNS_SUCCEEDED) return RecvRes;

	StrBuffer = (char*)malloc(TotalStringSizeInBytes * sizeof(char));
	if (StrBuffer == NULL) {
		printf("Error: Memory allocation receive buffer failed!\n");
		return TRNS_FAILED;
	}
	RecvRes = ReceiveBuffer(
		(char *)(StrBuffer),
		(int)(TotalStringSizeInBytes),
		sd);

	if (RecvRes == TRNS_SUCCEEDED) {
		*OutputStrPtr = StrBuffer;
	} else {
		free(StrBuffer);
	}

	return RecvRes;
}