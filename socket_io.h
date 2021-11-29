/**
 * @file socket_io.c
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Definitions for socket_io.c
 */

#ifndef SOCKET_IO
#define SOCKET_IO

#define LOCALHOST "LOCALHOST"
#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 81920
#define MAX_PORT 65535

/**
 * Creates socket and connects to server on localhost at specified port
 * 
 * @param  port specified port number
 * 
 * @return true if no errors were encountered, else false
 */
int connect_to_server(int);

/**
 * Configures socket address for client for connecting to localhost on specified port
 * 
 * @param  address sockaddr_in struct to configure
 * @param  port specified port number
 * 
 * @return true if no errors were encountered, else false
 */
bool setup_client_socket_addr(struct sockaddr_in *, int);

/**
 * Writes the specified string to the specified socket
 * 
 * @param  string_to_send string to write to socket
 * @param  socket_fd socket to write string to
 */
void send_string(char *, int);

/**
 * Creates a socket, binds it to specified port, and listens to it
 * 
 * @param  port specified port number
 * 
 * @return file descriptor of new listen socket
 */
int setup_listen_socket(int);

/**
 * Configures socket address for server for connecting to localhost on specified port
 * 
 * @param  address sockaddr_in struct to configure
 * @param  port specified port number
 * 
 * @return true if no errors were encountered, else false
 */
void setup_server_socket_addr(struct sockaddr_in *, int);

#endif