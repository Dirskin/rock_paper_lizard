#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdbool.h> 
#include <stdio.h>
#include <string.h>

#include "../shared/socket_shared.h"
#include "SocketSendRecvTools.h"
#define MAX_MSG_LEN_ZERO_PARAMS 29 //"SERVER_PLAYER_MOVE_REQUEST"(26) +":"(1) +"\n" +"\0"  

e_Msg_Type identify_msg_type(char *msg_type) {
	if (STRINGS_ARE_EQUAL(msg_type, "CLIENT_REQUEST")) {
		return CLIENT_REQUEST;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "CLIENT_MAIN_MENU")) {
		return CLIENT_MAIN_MENU;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "CLIENT_CPU")) {
		return CLIENT_CPU;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "CLIENT_VERSUS")) {
		return CLIENT_VERSUS;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "CLIENT_LEADERBOARD")) {
		return CLIENT_LEADERBOARD;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "CLIENT_PLAYER_MOVE")) {
		return CLIENT_PLAYER_MOVE;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "CLIENT_REPLAY")) {
		return CLIENT_REPLAY;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "CLIENT_REFRESH")) {
		return CLIENT_REFRESH;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "CLIENT_DISCONNECT")) {
		return CLIENT_DISCONNECT;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "SERVER_MAIN_MENU")) {
		return SERVER_MAIN_MENU;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "SERVER_APPROVED")) {
		return SERVER_APPROVED;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "SERVER_DENIED")) {
		return SERVER_DENIED;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "SERVER_INVITE")) {
		return SERVER_INVITE;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "SERVER_PLAYER_MOVE_REQUEST")) {
		return SERVER_PLAYER_MOVE_REQUEST;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "SERVER_GAME_RESULTS")) {
		return SERVER_GAME_RESULTS;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "SERVER_GAME_OVER_MENU")) {
		return SERVER_GAME_OVER_MENU;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "SERVER_OPPONENT_QUIT")) {
		return SERVER_OPPONENT_QUIT;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "SERVER_NO_OPPONENTS")) {
		return SERVER_NO_OPPONENTS;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "SERVER_LEADERBOARD")) {
		return SERVER_LEADERBOARD;
	}
	if (STRINGS_ARE_EQUAL(msg_type, "SERVER_LEADERBOARD_MENU")) {
		return SERVER_LEADERBOARD_MENU;
	}
	return ERR;
}

/*this function sends messages with zero params only*/
TransferResult_t send_msg_zero_params(e_Msg_Type msg_type, SOCKET t_socket) {
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
		strcpy(sendbuf, "SERVER_NO_OPPONENTS:\n");
		break;
	case SERVER_LEADERBOARD_MENU:
		strcpy(sendbuf, "SERVER_LEADERBOARD_MENU:\n");
		break;
	case SERVER_GAME_OVER_MENU:
		strcpy(sendbuf, "SERVER_GAME_OVER_MENU:\n");
		break;
	case SERVER_OPPONENT_QUIT:
		strcpy(sendbuf, "SERVER_OPPONENT_QUIT:\n");
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
	case CLIENT_REPLAY:
		strcpy(sendbuf, "CLIENT_REPLAY:\n");
		break;
	case CLIENT_REFRESH:
		strcpy(sendbuf, "CLIENT_REFRESH:\n");
		break;
	case CLIENT_DISCONNECT:
		strcpy(sendbuf, "CLIENT_DISCONNECT:\n");
		break;
	}
	res = SendString(sendbuf, t_socket);
	if (res == TRNS_FAILED) {
		printf("Service socket error while writing, closing thread.\n");
		return ERR_SOCKET_SEND;
	}
	return res;
}

/*this function sends messages with one param only*/
TransferResult_t send_msg_one_param(e_Msg_Type msg_type, SOCKET t_socket, char *param_1) {
	char *sendbuf;
	TransferResult_t res;
	sendbuf = (char *)malloc(MAX_MSG_LEN_ZERO_PARAMS + strlen(param_1));
	if (sendbuf == NULL) {
		printf("Error: memory allocation failed in send-buffer\n");
		return false;
	}
	switch (msg_type) {
	case CLIENT_REQUEST:
		sprintf(sendbuf, "CLIENT_REQUEST:%s\n", param_1);
		break;
	case CLIENT_PLAYER_MOVE:
		sprintf(sendbuf, "CLIENT_PLAYER_MOVE:%s\n", param_1);
		break;
	case SERVER_DENIED:
		sprintf(sendbuf, "SERVER_DENIED:%s\n", param_1);
		break;
	case SERVER_INVITE:
		sprintf(sendbuf, "SERVER_INVITE:%s\n", param_1);
		break;
	case SERVER_OPPONENT_QUIT:
		sprintf(sendbuf, "SERVER_OPPONENT_QUIT:%s\n", param_1);
		break;
	}
	res = SendString(sendbuf, t_socket);
	return res;
}

/*this function sends messages with four parameters*/
TransferResult_t send_msg_quad_params(e_Msg_Type msg_type, SOCKET t_socket, char *param_1, char *param_2, char *param_3, char *param_4) {
	char *sendbuf;
	TransferResult_t res;
	sendbuf = (char *)malloc(MAX_MSG_LEN_ZERO_PARAMS + strlen(param_1) + strlen(param_2) + strlen(param_3) + strlen(param_4));
	if (!sendbuf)
		return ERR_MALLOC;

	switch (msg_type) {
	case SERVER_GAME_RESULTS:
		sprintf(sendbuf, "SERVER_GAME_RESULTS:%s;%s;%s;%s\n", param_1, param_2, param_3, param_4);
		break;
	}

	res = SendString(sendbuf, t_socket);

	if (res == TRNS_FAILED) {
		printf("Service socket error while writing, closing thread.\n");
		return ERR_SOCKET_SEND;
	}
	return res;
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
		if (BytesTransferred == SOCKET_ERROR) {
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
		if (BytesJustTransferred == SOCKET_ERROR) {
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

/*input: message as string. output: RX_msg struct, contains the original message divided to params*/
RX_msg* parse_message_params(char *message) {
	int i, curr_param_len, last_semicolon_ind = 0, arg_index = 1;
	RX_msg *parsed_msg;
	char msg_type[MAX_MSG_TYPE_LEN], *colon_ind;

	parsed_msg = (RX_msg*)malloc(sizeof(RX_msg));
	parsed_msg->arg_1 = NULL;
	parsed_msg->arg_2 = NULL;
	parsed_msg->arg_3 = NULL;
	parsed_msg->arg_4 = NULL;

	if (!parsed_msg) {
		goto err_type;
	}
	colon_ind = strchr(message, ':');
	curr_param_len = strlen(message) - strlen(colon_ind);
	strncpy(msg_type, message, curr_param_len);
	msg_type[curr_param_len] = '\0';					/* copied partial string, must finish it with \0 */
	parsed_msg->msg_type = identify_msg_type(msg_type);

	i = curr_param_len;
	last_semicolon_ind = curr_param_len;
	while (message[i] != '\0') {
		if (message[i] == ';' || message[i] == '\n') {
			curr_param_len = i - last_semicolon_ind;
			switch (arg_index) {
			case 1:
				parsed_msg->arg_1 = (char *)malloc(curr_param_len+1);
				if (!parsed_msg->arg_1)
					goto err_arg1;
				strncpy(parsed_msg->arg_1, &message[last_semicolon_ind +1], curr_param_len-1);
				parsed_msg->arg_1[curr_param_len-1] = '\0';
				break;
			case 2:
				parsed_msg->arg_2 = (char *)malloc(curr_param_len + 1);
				if (!parsed_msg->arg_2)
					goto err_arg2;
				strncpy(parsed_msg->arg_2, &message[last_semicolon_ind +1], curr_param_len-1);
				parsed_msg->arg_2[curr_param_len-1] = '\0';
				break;
			case 3:
				parsed_msg->arg_3 = (char *)malloc(curr_param_len + 1);
				if (!parsed_msg->arg_3)
					goto err_arg3;
				strncpy(parsed_msg->arg_3, &message[last_semicolon_ind+1], curr_param_len-1);
				parsed_msg->arg_3[curr_param_len-1] = '\0';
				break;
			case 4:
				parsed_msg->arg_4 = (char *)malloc(curr_param_len + 1);
				if (!parsed_msg->arg_4)
					goto err_arg4;
				strncpy(parsed_msg->arg_4, &message[last_semicolon_ind +1], curr_param_len-1);
				parsed_msg->arg_4[curr_param_len-1] = '\0';
				break;
			}
			last_semicolon_ind = i;
			arg_index++;
		}
		i++;
	}
	return parsed_msg;

err_arg4:
	free(parsed_msg->arg_3);
err_arg3:
	free(parsed_msg->arg_2);
err_arg2:
	free(parsed_msg->arg_1);
err_arg1:
	free(parsed_msg);
err_type:
	printf("Error allocating memory for RX_msg\n");
	return NULL;

}