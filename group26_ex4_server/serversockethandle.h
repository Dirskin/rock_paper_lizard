#ifndef SOCKET_EXAMPLE_SERVER_H
#define SOCKET_EXAMPLE_SERVER_H


#include <winsock2.h>
#include "../shared/common.h"

int MainServer(int port);
int get_response(RX_msg **rx_msg, SOCKET *t_socket);

#endif // SOCKET_EXAMPLE_SERVER_H