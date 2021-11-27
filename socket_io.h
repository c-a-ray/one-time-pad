#ifndef SOCKET_IO
#define SOCKET_IO

#define LOCALHOST "LOCALHOST"
#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 4096
#define MAX_PORT 65535

bool setup_client_socket_addr(struct sockaddr_in *, int);
int connect_to_server(int);
void send_string(char *, int);
int setup_listen_socket(int);
void setup_server_socket_addr(struct sockaddr_in *, int);

#endif