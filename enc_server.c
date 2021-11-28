/*
Usage: enc_server <port>
Performs encryption via one-time pad
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <stdbool.h>

#include "socket_io.h"
#include "util.h"

struct Args 
{
    char *plaintext;
    char *key;
    char *ciphertext;
};

int get_port(int, char **);
void handle_SIGCHLD(int);
bool catch_SIGCHLD(void);
void handle_connection(int);
bool perform_handshake(int);
void encrypt(struct Args);

// Number of currently running processes
int n_connections = 0;

int main(int argc, char **argv)
{
    // Get and validate port
    int port = get_port(argc, argv);
    if (port == 0)
        return EXIT_FAILURE;

    // Set up listening socket
    int listen_socket_fd = setup_listen_socket(port);
    if (listen_socket_fd < 0)
        return EXIT_FAILURE;

    // Setup SIGCHLD signal handler to clean up children on termination
    if (!catch_SIGCHLD())
        return EXIT_FAILURE;

    // Setup client socket address
    struct sockaddr_in client_addr;
    socklen_t client_socklen = sizeof(client_addr);

    // Process connections
    while (true)
    {
        // Accept connection and check for error
        int socket_fd = accept( listen_socket_fd, (struct sockaddr *) &client_addr, &client_socklen );
        if (socket_fd < 0)
        {
            fprintf(stderr, "Error: failed to accept connection");
            return EXIT_FAILURE;
        }

        n_connections++;

        pid_t pid = fork();
        switch (pid)
        {
            case -1:
                fprintf(stderr, "Error: fork() failed\n");
                exit(EXIT_FAILURE);
            case 0:
                if (n_connections > MAX_CONNECTIONS)
                    exit(EXIT_SUCCESS);

                close(listen_socket_fd);
                handle_connection(socket_fd);
                exit(EXIT_SUCCESS);
            default:
                close(socket_fd);
        }
    }

    return EXIT_SUCCESS;
}

int get_port(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Error: missing argument\n");
        fprintf(stderr, "Usage: enc_server $port");
        return 0;
    }

    int port = atoi(argv[1]);
    if (port < 1 || port > MAX_PORT)
    {
        fprintf(stderr, "Error: invalid port: %s\n", port);
        return 0;
    }
    
    return port;
}

void handle_SIGCHLD(int signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
        n_connections--;
}

bool catch_SIGCHLD(void)
{
    struct sigaction sa_SIGCHLD;
    sa_SIGCHLD.sa_handler = handle_SIGCHLD;
    sigemptyset(&sa_SIGCHLD.sa_mask);
    sa_SIGCHLD.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa_SIGCHLD, NULL) == -1)
    {
        fprintf(stderr, "Error: failed to setup handler for child termination signal\n");
        return false;
    }
    return true;
}

void handle_connection(int socket_fd)
{
    struct Args args;

    // Verify that connection is to enc_client
    if (!perform_handshake(socket_fd))
        return;

    // Get plaintext and key from enc_client
    char *buffer = (char *) malloc(BUFFER_SIZE);
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

bool perform_handshake(int socket_fd)
{
    const char *enc_client_signal = "enc_client@";
    int enc_client_sig_len = strlen(enc_client_signal);

    char *buffer = (char *) malloc(enc_client_sig_len + 1);
    memset(buffer, '\0', enc_client_sig_len + 1);

    int n_read = recv(socket_fd, buffer, enc_client_sig_len, 0);

    bool success = true;
    if (n_read < enc_client_sig_len || strcmp(buffer, enc_client_signal) != 0)
        success = false;

    send_string("enc_server", socket_fd);
    free(buffer);
    return success;
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