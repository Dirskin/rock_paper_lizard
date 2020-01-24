
#ifndef SOCKET_EXAMPLE_CLIENT_H
#define SOCKET_EXAMPLE_CLIENT_H


/* F = Flags, for flags*/
#define F_SERVER_CONNECTION_FAILED	((int)(-1012))
#define F_SERVER_CONNECTION_LOST	((int)(-1013))
#define F_SERVER_DENIED_CONNECTION	((int)(-1014))

#define RESPONSE_MAX_WAIT_TIME 5000

/*Function declerations*/
int MainClient(const char *server_ip_addr, int server_port, char *username);


#endif // SOCKET_EXAMPLE_CLIENT_H