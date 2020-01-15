
#ifndef SOCKET_EXAMPLE_CLIENT_H
#define SOCKET_EXAMPLE_CLIENT_H

int MainClient(const char *server_ip_addr, int port);

/* F = Flags, for flags*/
#define F_SERVER_CONNECTION_FAILED	101112
#define F_SERVER_CONNECTION_LOST	101113
#define F_SERVER_DENIED_CONNECTION	101114
#endif // SOCKET_EXAMPLE_CLIENT_H