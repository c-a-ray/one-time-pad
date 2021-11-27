#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netdb.h>
#include <stdbool.h>

#include "socket_io.h"

bool setup_client_socket_addr(struct sockaddr_in *address, int port)
{
    memset( (char*) address, '\0', sizeof(*address) );
    
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    struct hostent *host_info = gethostbyname(LOCALHOST);
    if (host_info == NULL)
    {
        fprintf(stderr, "Error: host not found\n");
        return false;
    }

    memcpy( (char *) &address->sin_addr.s_addr, host_info->h_addr_list[0], host_info->h_length );
    return true;
}

int connect_to_server(int port)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (socket_fd < 0)
        return -1;

    struct sockaddr_in server_addr;
    if (!setup_client_socket_addr(&server_addr, port))
        return -1;

    if ( connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) )
        return -1;

    return socket_fd; 
}

void send_string(char *string_to_send, int socket_fd)
{
    int msg_len = strlen(string_to_send) + 1;
    char *message = (char *) malloc(msg_len);
    memset(message, '\0', msg_len);
    strcpy(message, string_to_send);
    strcat(message, "@");

    int n_written = 0;
    int total_written = 0;

    do
    {
        n_written = send(socket_fd, message, strlen(message), 0);
        if (n_written < 0)
        {
            fprintf(stderr, "Error: failed to write to socket\n");
            break;
        }

        total_written += n_written;
        if (total_written < msg_len)
        {
            free(message);
            message = (char *) malloc(msg_len - total_written);
            memset(message, '\0', msg_len - total_written);
            strcpy(message, &string_to_send[total_written - 1]);
        }
    } while (total_written < msg_len);
}

int setup_listen_socket(int port)
{
    struct sockaddr_in server_addr;
    setup_server_socket_addr(&server_addr, port);

    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
    {
        fprintf(stderr, "Error: failed to open listening socket\n");
        return -1;
    }
    
    if ( bind( listen_socket, (struct sockaddr *) &server_addr, sizeof(server_addr) ) < 0 )
    {
        fprintf(stderr, "Error: failed to bind socket to port\n");
        return -1;
    }
    
    listen(listen_socket, MAX_CONNECTIONS);
    return listen_socket;
}

void setup_server_socket_addr(struct sockaddr_in *address, int port)
{
    memset( (char *) address, '\0', sizeof(*address) );
    
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    address->sin_addr.s_addr = INADDR_ANY;
}

