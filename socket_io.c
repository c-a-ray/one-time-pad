/**
 * @file socket_io.c
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Contains shared functions used for setting up, connecting, and writing to a socket.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netdb.h>
#include <stdbool.h>

#include "socket_io.h"

int connect_to_server(int port)
{
    // Create socket to use IPv4 and TCP
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (socket_fd < 0)
        return -1;

    // Create and configure address struct
    struct sockaddr_in server_addr;
    if (!setup_client_socket_addr(&server_addr, port))
        return -1;

    // Connect to the server
    if ( connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) )
        return -1;

    return socket_fd; 
}

bool setup_client_socket_addr(struct sockaddr_in *address, int port)
{
    // Zero out address struct
    memset( (char *) address, '\0', sizeof(*address));
    
    // Make address network capable
    address->sin_family = AF_INET;

    // Set port number
    address->sin_port = htons(port);

    // Get IP address of localhost
    struct hostent *host_info = gethostbyname(LOCALHOST);
    if (host_info == NULL)
    {
        fprintf(stderr, "Error: host not found\n");
        return false;
    }

    // Copy localhost IP address to sin_addr.s_addr
    memcpy( (char *) &address->sin_addr.s_addr, host_info->h_addr_list[0], host_info->h_length );
    return true;
}

void send_string(char *string_to_send, int socket_fd)
{
    // Create message to be sent
    int msg_len = strlen(string_to_send) + 1;
    char *message = (char *) malloc(msg_len);
    memset(message, '\0', msg_len);

    // Copy string to send to message
    strcpy(message, string_to_send);

    // Append stop character to end of message
    strcat(message, "@");

    int n_written = 0;          // Number of characters sent per send()
    int total_written = 0;      // Total number of message characters written

    // Send the message
    do
    {
        // Send chunk of message
        n_written = send(socket_fd, message, strlen(message), 0);
        if (n_written < 0)
        {
            fprintf(stderr, "Error: failed to write to socket\n");
            break;
        }

        // Keep track of total number of characters sent
        total_written += n_written;

        // Check if the entire message was sent
        if (total_written < msg_len)
        {
            // Replace message with the unsent portion of the message
            free(message);
            message = (char *) malloc(msg_len - total_written);
            memset(message, '\0', msg_len - total_written);
            strcpy(message, &string_to_send[total_written - 1]);
        }
    } while (total_written < msg_len); // Iterate until entire message has been sent
}

int setup_listen_socket(int port)
{
    // Create and configure address struct
    struct sockaddr_in server_addr;
    setup_server_socket_addr(&server_addr, port);

    // Create socket to listen to
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
    {
        fprintf(stderr, "Error: failed to open listening socket\n");
        return -1;
    }
    
    // Bind listening socket to specified port
    if ( bind( listen_socket, (struct sockaddr *) &server_addr, sizeof(server_addr) ) < 0 )
    {
        fprintf(stderr, "Error: port %d is unavailable\n", port);
        return -1;
    }
    
    // Listen to the socket and return its file descriptor
    listen(listen_socket, MAX_CONNECTIONS);
    return listen_socket;
}

void setup_server_socket_addr(struct sockaddr_in *address, int port)
{
    // Zero out address struct
    memset( (char *) address, '\0', sizeof(*address) );
    
    // Make address network-capable
    address->sin_family = AF_INET;

    // Set port number
    address->sin_port = htons(port);

    // Allow connection to any address
    address->sin_addr.s_addr = INADDR_ANY;
}

