/**
 * @file dec_server.c
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * dec_server listens on specified port for connections from dec_client.
 * Each new connection is run in a separate process. Five processes can run at a time.
 * When ciphertext and key are received, dec_server decrypts the ciphertext using
 * one-time-pad and sends plaintext to dec_client.
 * 
 * Usage: dec_server <port>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <stdbool.h>

#include "dec_server.h"
#include "socket_io.h"
#include "util.h"

// Number of currently running processes
int n_connections = 0;

int main(int argc, char **argv)
{
    // Validate port and convert it to an integer
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

    // Continuously process connections
    while (true)
    {
        // Accept new connection and check for error
        int socket_fd = accept( listen_socket_fd, (struct sockaddr *) &client_addr, &client_socklen );
        if (socket_fd < 0)
        {
            fprintf(stderr, "Error: failed to accept connection");
            return EXIT_FAILURE;
        }

        // Increment the number of connections
        n_connections++;

        // Create a new process
        pid_t pid = fork();
        switch (pid)
        {
            case -1: // Fork failed
                fprintf(stderr, "Error: fork() failed\n");
                exit(EXIT_FAILURE);

            case 0: // Child process
                // Return without handling connection if max connections reached
                if (n_connections > MAX_CONNECTIONS)
                    exit(EXIT_SUCCESS);

                // Close the listening socket
                close(listen_socket_fd);

                // Handle the connection
                handle_connection(socket_fd);
                exit(EXIT_SUCCESS);

            default: // Parent process
                // Close the new connection
                close(socket_fd);
        }
    }

    return EXIT_SUCCESS;
}

int get_port(int argc, char **argv)
{
    // Verify the correct number of arguments was given
    if (argc < 2)
    {
        fprintf(stderr, "Error: missing argument\n");
        fprintf(stderr, "Usage: dec_server $port");
        return 0;
    }

    // Convert port to an integer
    int port = atoi(argv[1]);

    // Verify that specified port is a valid port number
    if (port < 1 || port > MAX_PORT)
    {
        fprintf(stderr, "Error: invalid port: %s\n", port);
        return 0;
    }
    
    // Return the port number as an integer
    return port;
}

void handle_SIGCHLD(int signo)
{
    // Perform non-blocking wait for any child process
    // When child terminates, decrement the number of connections
    while (waitpid(-1, NULL, WNOHANG) > 0);
        n_connections--;
}

bool catch_SIGCHLD(void)
{
    // Declare SIGCHLD action struct
    struct sigaction sa_SIGCHLD;

    // Register handle_SIGCHLD as signal handler
    sa_SIGCHLD.sa_handler = handle_SIGCHLD;

    // Initialize signal mask to exclude all signals
    sigemptyset(&sa_SIGCHLD.sa_mask);

    // Set flag to cause primitive library functions to resume after handler returns
    // See https://www.gnu.org/software/libc/manual/html_node/Flags-for-Sigaction.html
    sa_SIGCHLD.sa_flags = SA_RESTART;

    // Install signal handler and check for error
    if (sigaction(SIGCHLD, &sa_SIGCHLD, NULL) == -1)
    {
        fprintf(stderr, "Error: failed to setup handler for child termination signal\n");
        return false;
    }
    return true;
}

void handle_connection(int socket_fd)
{
    // Declare object to hold ciphertext, key, and plaintext
    struct Args args;

    // Verify that connection is to dec_client
    if (!perform_handshake(socket_fd))
        return;

    // Create buffer for reading from socket
    char *buffer = (char *) malloc(BUFFER_SIZE);
    memset(buffer, '\0', BUFFER_SIZE);

    // Create string to store entire message
    int full_string_size = BUFFER_SIZE;
    char *full_recd_string = (char *) malloc(full_string_size);
    memset(full_recd_string, '\0', full_string_size);

    int n_read = 0;                 // Number of characters read into buffer
    int total_n_read = 0;           // Total number of characters so far in message
    int stop_idx_1, stop_idx_2;     // Indices of stop characters
    do
    {
        // Read from socket, filling buffer
        n_read = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0);
        if (n_read < 0)
            fprintf(stderr, "Error: failed to read from socket\n");

        // Keep track of total number of characters read
        total_n_read += n_read;

        // If the message is larger than full_recd_string, resize full_recd_string
        if ((float) total_n_read / (float) full_string_size > 1)
        {            
            full_string_size *= 2;
            full_recd_string = (char *) realloc(full_recd_string, full_string_size);
        }

        // Add contents of buffer to full_recd_string
        strcat(full_recd_string, buffer);

        // Zero out buffer for re-use
        memset(buffer, '\0', BUFFER_SIZE);

        // Search for stop characters
        find_stop_indices(full_recd_string, &stop_idx_1, &stop_idx_2);

    } while (stop_idx_1 == -1 || stop_idx_2 == -1); // Iterate until both stop characters are found

    // Extract ciphertext from message
    args.ciphertext = (char *) malloc(stop_idx_1 + 1);
    memset(args.ciphertext, '\0', stop_idx_1 + 1);
    strncpy(args.ciphertext, full_recd_string, stop_idx_1);

    // Extract key from message
    args.key = (char *) malloc(stop_idx_2 - stop_idx_1 + 1);
    memset(args.key, '\0', stop_idx_2 - stop_idx_1 + 1);
    strncpy(args.key, &full_recd_string[stop_idx_1 + 1], stop_idx_2 - stop_idx_1 - 1);

    // Free allocated memory
    free(buffer);
    free(full_recd_string);

    // Create plaintext
    args.plaintext = (char *) malloc(strlen(args.ciphertext) + 1);
    memset(args.plaintext, '\0', strlen(args.ciphertext) + 1);
    decrypt(args);

    // Send plaintext
    send_string(args.plaintext, socket_fd);
}

bool perform_handshake(int socket_fd)
{
    // dec_client identifier (expected value)
    const char *dec_client_signal = "dec_client@";
    int dec_client_sig_len = strlen(dec_client_signal);

    // Create buffer to read from socket
    char *buffer = (char *) malloc(dec_client_sig_len + 1);
    memset(buffer, '\0', dec_client_sig_len + 1);

    // Read from socket
    int n_read = recv(socket_fd, buffer, dec_client_sig_len, 0);
    
    // Verify that the client identified itself as dec_client
    bool success = true;
    if (n_read < dec_client_sig_len || strcmp(buffer, dec_client_signal) != 0)
        success = false;

    // Identify self as dec_server to client
    send_string("dec_server", socket_fd);

    // Free allocated memory
    free(buffer);
    return success;
}

void decrypt(struct Args args)
{
    // Walk through all characters in ciphertext
    for (int i = 0; i < strlen(args.ciphertext); i++)
    {
        // Convert encrypted character to integer between 0 and 26
        int cipher_val = args.ciphertext[i] == ' ' ? 0 : args.ciphertext[i] - 64;

        // Convert key character at same index to integer between 0 and 26
        int key_val = args.key[i] == ' ' ? 0 : args.key[i] - 64;

        // Subtract key value from encrypted value and mod by 27
        int plain_val = mod(cipher_val - key_val, 27);

        // Convert decrypted value to character and store in plaintext at current index
        args.plaintext[i] = plain_val == 0 ? ' ' : plain_val + 64;
    }
}