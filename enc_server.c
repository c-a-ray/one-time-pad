/*
Usage: enc_server <port>
Performs encryption via one-time pad

    - Listens on specified port
    - Outputs error when executed if it can't run due to a network error like unavailable port
    - When connection is made:
        - Calls accept() to generate socket used for communication 
        - Uses a separate process the rest of the servicing for this connection on the new socket
        - Child process verifies it is communicating with enc_client
        - Child receives plaintext and a key from enc_client
        - Child sends the ciphertext back to enc_client
    - Must support up to 5 concurrent socket connections
    - Outputs errors to stderr and continues to run
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#define MAX_PORT 65535
#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 4096
#define STOP_SIGNAL '@'

struct Args 
{
    char *plaintext;
    char *key;
    char *ciphertext;
};

int get_port(int, char **);
int setup_listen_socket(int);
void error_and_exit(char []);
void setup_socket_addr(struct sockaddr_in *, int);
void find_stop_indices(const char *, int *, int *);
void send_string(char *, int);
void handle_connection(int);
void encrypt(struct Args);

int main(int argc, char **argv)
{
    // Setup listening socket on user-specified port    
    int listen_socket_fd = setup_listen_socket(get_port(argc, argv));

    int n_connections = 0;
    struct sockaddr_in client_addr;
    socklen_t client_socklen = sizeof(client_addr);

    while (true)
    {
        // Accept connection
        int socket_fd = accept( listen_socket_fd, (struct sockaddr *) &client_addr, &client_socklen );
        if (socket_fd < 0)
            error_and_exit("Error: failed to accept connection");

        n_connections++;
        pid_t pid = fork();
        if (pid < 0)
            fprintf(stderr, "Error: fork() failed\n");
        else if (pid > 0)
            close(socket_fd);
        else
        {
            if (n_connections > MAX_CONNECTIONS)
                exit(EXIT_SUCCESS);
                
            close(listen_socket_fd);
            handle_connection(socket_fd);
            exit(EXIT_SUCCESS);
        }
    }

    return EXIT_SUCCESS;
}

int get_port(int argc, char **argv)
{
    if (argc < 2)
        error_and_exit("Usage: enc_server $port");

    int port = atoi(argv[1]);
    if (port < 1 || port > MAX_PORT)
        error_and_exit("Error: a valid port number must be specified");
    
    return port;
}

int setup_listen_socket(int port)
{
    struct sockaddr_in server_addr;
    setup_socket_addr(&server_addr, port);

    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
        error_and_exit("Error: failed to open socket");
    
    if ( bind( listen_socket, (struct sockaddr *) &server_addr, sizeof(server_addr) ) < 0 )
        error_and_exit("Error: failed to bind socket to port");
    
    listen(listen_socket, MAX_CONNECTIONS);
    return listen_socket;
}

void error_and_exit(char err[])
{
    fprintf(stderr, "%s\n", err);
    exit(EXIT_FAILURE);
}

void setup_socket_addr(struct sockaddr_in *address, int port)
{
    memset( (char *) address, '\0', sizeof(*address) );
    
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    address->sin_addr.s_addr = INADDR_ANY;
}

void find_stop_indices(const char *buffer, int *stop_idx_1, int *stop_idx_2)
{
    *stop_idx_1 = -1;
    *stop_idx_2 = -1;

    for (int i = 0; i < strlen(buffer); i++)
    {
        if (buffer[i] == '@')
        {
            if (*stop_idx_1 == -1)
                *stop_idx_1 = i;
            else if (*stop_idx_2 == -1)
            {
                *stop_idx_2 = i;
                break;
            }
        }
    }
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

void handle_connection(int socket_fd)
{
    struct Args args;

    // Verify that connection is to enc_client
    const char *enc_client_signal = "enc_client@";
    int enc_client_sig_len = strlen(enc_client_signal);

    char *buffer = (char *) malloc(enc_client_sig_len + 1);
    memset(buffer, '\0', enc_client_sig_len + 1);

    int n_read = recv(socket_fd, buffer, enc_client_sig_len, 0);
    if (n_read < enc_client_sig_len || strcmp(buffer, enc_client_signal) != 0)
        error_and_exit("Handshake failed. Invalid connection.\n");

    send_string("enc_server", socket_fd);

    // Get plaintext and key from enc_client
    free(buffer);
    buffer = (char *) malloc(BUFFER_SIZE);
    memset(buffer, '\0', BUFFER_SIZE);

    int full_string_size = BUFFER_SIZE;
    char *full_recd_string = (char *) malloc(full_string_size);
    memset(full_recd_string, '\0', full_string_size);

    int total_n_read = 0;
    int stop_idx_1, stop_idx_2;
    do
    {
        int n_read = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0);
        if (n_read < 0)
            fprintf(stderr, "Error: failed to read from socket\n");

        total_n_read += n_read;

        if ((float) total_n_read / (float) full_string_size > 1)
        {            
            full_string_size *= 2;
            full_recd_string = (char *) realloc(full_recd_string, full_string_size);
        }

        strcat(full_recd_string, buffer);
        memset(buffer, '\0', BUFFER_SIZE);

        find_stop_indices(full_recd_string, &stop_idx_1, &stop_idx_2);
    } while (stop_idx_1 == -1 || stop_idx_2 == -1);
    
    free(buffer);

    // Extract plaintext from sent data
    args.plaintext = (char *) malloc(stop_idx_1 + 1);
    memset(args.plaintext, '\0', stop_idx_1 + 1);
    strncpy(args.plaintext, full_recd_string, stop_idx_1);

    // Extract key from sent data
    args.key = (char *) malloc(stop_idx_2 - stop_idx_1 + 1);
    memset(args.key, '\0', stop_idx_2 - stop_idx_1 + 1);
    strncpy(args.key, &full_recd_string[stop_idx_1 + 1], stop_idx_2 - stop_idx_1 - 1);

    free(full_recd_string);

    // Create ciphertext
    args.ciphertext = (char *) malloc(strlen(args.plaintext) + 1);
    memset(args.ciphertext, '\0', strlen(args.plaintext) + 1);
    encrypt(args);

    // Send ciphertext
    send_string(args.ciphertext, socket_fd);
}

void encrypt(struct Args args)
{
    for (int i = 0; i < strlen(args.plaintext); i++)
    {
        int plain_val = args.plaintext[i] == ' ' ? 0 : args.plaintext[i] - 64;
        int key_val = args.key[i] == ' ' ? 0 : args.key[i] - 64;
        int ciphertext_val = (plain_val + key_val) % 27;
        args.ciphertext[i] = ciphertext_val == 0 ? ' ' : ciphertext_val + 64;
    }   
}