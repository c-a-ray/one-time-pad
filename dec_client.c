/**
 * @file dec_client.c
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Connects and sends ciphertext and key to dec_server at specified port.
 * If dec_server is not running on specified port, connection is refused.
 * After dec_server responds with plaintext, plaintext is written to stdout.
 * 
 * Usage: dec_client <plaintext> <key> <port>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdbool.h>

#include "dec_client.h"
#include "socket_io.h"
#include "util.h"

int main(int argc, char *argv[])
{
    // Get command line arguments
    if (argc < 4)
    {
        fprintf(stderr, "Error: missing %d arguments\n", 4 - argc);
        return EXIT_FAILURE;
    }

    // Store values provided by user
    struct Config cfg = {
        ciphertext_filename: argv[1],
        key_filename: argv[2],
        port: atoi(argv[3]),
    };

    // Declare object to hold plaintext, key and ciphertext
    struct Args args;

    // Open ciphertext file
    FILE *fp_ciphertext = fopen(cfg.ciphertext_filename, "rb");
    if (fp_ciphertext == 0)
    {
        fprintf(stderr, "Error: failed to open ciphertext file \"%s\"\n", cfg.ciphertext_filename);
        return EXIT_FAILURE;
    }

    // Determine length of ciphertext
    fseek(fp_ciphertext, 0, SEEK_END);
    long f_size = ftell(fp_ciphertext);
    fseek(fp_ciphertext, 0, SEEK_SET);

    // Read ciphertext from file and store in args
    args.ciphertext = (char *) malloc(f_size + 1);
    fread(args.ciphertext, f_size, 1, fp_ciphertext);
    fclose(fp_ciphertext);

    // Open key file
    FILE *fp_key = fopen(cfg.key_filename, "rb");
    if (fp_key == 0)
    {
        fprintf(stderr, "Error: failed to open key file \"%s\"\n", cfg.key_filename);
        return EXIT_FAILURE;
    }

    // Determine key length
    fseek(fp_key, 0, SEEK_END);
    f_size = ftell(fp_key);
    fseek(fp_key, 0, SEEK_SET);

    // Read key from file and store in args
    args.key = (char *) malloc(f_size + 1);
    fread(args.key, f_size, 1, fp_key);
    fclose(fp_key);

    // Replace trailing newline with NULL character in ciphertext and key
    replace_newline(args.ciphertext);
    replace_newline(args.key);

    // Verify there are no invalid characters in ciphertext
    char invalid_char = validate_message(args.ciphertext);
    if (invalid_char != 0)
    {
        fprintf(stderr, "Error: invalid character in ciphertext file: %c\n", invalid_char);
        return EXIT_FAILURE;
    }

    // Verify there are no invalid characters in ciphertext
    if (strlen(args.ciphertext) > strlen(args.key))
    {
        fprintf(stderr, "Error: ciphertext is longer than key\n");
        fprintf(stderr, "Ciphertext length: %d\tKey length: %d\n", strlen(args.ciphertext), strlen(args.key));
        return EXIT_FAILURE;
    }

    // Connect to server at specified port
    int socket_fd = connect_to_server(cfg.port);
    
    // Verify that connection is to dec_server
    if (!perform_handshake(socket_fd))
        return EXIT_FAILURE;

    // Send ciphertext and key to dec_server
    send_string(args.ciphertext, socket_fd);
    send_string(args.key, socket_fd);

    // Get plaintext from server and write to stdout
    get_plaintext_from_server(socket_fd, args);

    return EXIT_SUCCESS;
}

bool perform_handshake(int socket_fd)
{
    // Identify self to server
    send_string("dec_client", socket_fd);
    
    // Wait for server to identify itself
    char *handshake_response = malloc(BUFFER_SIZE);
    memset(handshake_response, '\0', BUFFER_SIZE);
    int n_read = recv(socket_fd, handshake_response, BUFFER_SIZE, 0);
    
    // If connected server is enc_server, refuse connection
    if ( strcmp(handshake_response, "enc_server@") == 0 )
    {
        fprintf(stderr, "Error: connection refused: dec_client cannot connect to enc_server\n");
        return false;
    }

    // If connected server is not recognized, refuse connection
    if ( strcmp(handshake_response, "dec_server@") != 0 )
    {
        fprintf(stderr, "Error: connection refused: unknown server: %s\n", handshake_response);
        return false;
    }

    // Return true if connected to dec_server
    return true;
}

void get_plaintext_from_server(int socket_fd, struct Args args)
{
    // Create a buffer for reading from socket
    char *buffer = (char *) malloc(BUFFER_SIZE);
    memset(buffer, '\0', BUFFER_SIZE);

    // Create a string to store the complete message
    int full_string_size = BUFFER_SIZE;
    char *full_recd_string = (char *) malloc(full_string_size);
    memset(full_recd_string, '\0', full_string_size);

    int n_read = 0;             // Number of characters read in recv()
    int total_n_read = 0;       // Total number of characters read
    int stop_idx;               // Index of stop character ('@')
    do
    {
        // Read from socket, filling buffer, and check for error
        n_read = recv(socket_fd, buffer, BUFFER_SIZE, 0);
        if (n_read < 0)
            fprintf(stderr, "Error: failed to read from socket\n");

        // Keep track of total number of characters read in current message
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

        // Search for stop character
        find_stop_index(full_recd_string, &stop_idx);

    } while (stop_idx == -1); // Iterate until stop character is found

    // Store full received string as plaintext
    args.plaintext = (char *) malloc(stop_idx + 1);
    memset(args.plaintext, '\0', stop_idx + 1);
    strncpy(args.plaintext, full_recd_string, stop_idx);

    // Free allocated memory
    free(buffer);
    free(full_recd_string);

    // Write plaintext to stdout
    fprintf(stdout, "%s\n", args.plaintext);
}