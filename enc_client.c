/**
 * @file enc_client.c
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Connects and sends plaintext and key to enc_server at specified port.
 * If enc_server is not running on specified port, connection is refused.
 * After enc_server responds with ciphertext, ciphertext is written to stdout.
 * 
 * Usage: enc_client <plaintext> <key> <port>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netdb.h>
#include <stdbool.h>

#include "enc_client.h"
#include "socket_io.h"
#include "util.h"

int main(int argc, char *argv[])
{
    // Get command line arguments
    if (argc < 4)
    {
        fprintf(stderr, "Missing %d arguments\n", 4 - argc);
        fprintf(stderr, "Usage: enc_client $plaintext $key $port\n");
        return EXIT_FAILURE;
    }

    struct Config cfg = {
        plaintext_filename: argv[1],
        key_filename: argv[2],
        port: atoi(argv[3]),
    };

    // Extract values from specified files
    struct Args args;

    // Get plaintext from file
    FILE *fp_plaintext = fopen(cfg.plaintext_filename, "rb");
    if (fp_plaintext == 0)
    {
        fprintf(stderr, "Error: failed to open plaintext file \"%s\"\n", cfg.plaintext_filename);
        return EXIT_FAILURE;
    }

    fseek(fp_plaintext, 0, SEEK_END);
    long f_size = ftell(fp_plaintext);
    fseek(fp_plaintext, 0, SEEK_SET);

    args.plaintext = (char *) malloc(f_size + 1);
    fread(args.plaintext, f_size, 1, fp_plaintext);
    fclose(fp_plaintext);

    // Get key from file
    FILE *fp_key = fopen(cfg.key_filename, "rb");
    if (fp_key == 0)
    {
        fprintf(stderr, "Error: failed to open key file \"%s\"\n", cfg.key_filename);
        return EXIT_FAILURE;
    }

    fseek(fp_key, 0, SEEK_END);
    f_size = ftell(fp_key);
    fseek(fp_key, 0, SEEK_SET);

    args.key = (char *) malloc(f_size + 1);
    fread(args.key, f_size, 1, fp_key);
    fclose(fp_key);

    // Replace newline with null character in plaintext and check for invalid characters
    replace_newline(args.plaintext);
    char invalid_char = validate_message(args.plaintext);
    if (invalid_char != 0)
    {
        fprintf(stderr, "Error: invalid character in plaintext file \"%s\": %c\n", cfg.plaintext_filename, invalid_char);
        return EXIT_FAILURE;
    }

    // Replace newline with null character in key
    replace_newline(args.key);

    // Make sure key is long enough
    if (strlen(args.plaintext) > strlen(args.key))
    {
        fprintf(stderr, "Error: plaintext is longer than key\n");
        fprintf(stderr, "Plaintext length: %d\tKey length: %d\n", strlen(args.plaintext), strlen(args.key));
        return EXIT_FAILURE;
    }

    // Connect to enc_server
    int socket_fd = connect_to_server(cfg.port);
    if (socket_fd < 0)
    {
        fprintf(stderr, "Error: failed to connect to server at port %d\n", cfg.port);
        return EXIT_FAILURE;
    }
    
    if (!perform_handshake(socket_fd))
    {
        fprintf(stderr, "Error: connection refused\n");
        return EXIT_FAILURE;
    }

    // Send plaintext and key to enc_server
    send_string(args.plaintext, socket_fd);
    send_string(args.key, socket_fd);

    // Get ciphertext from server and write to stdout
    get_ciphertext_from_server(socket_fd, args);

    return EXIT_SUCCESS;
}

bool perform_handshake(int socket_fd)
{
    send_string("enc_client", socket_fd);
    
    char *handshake_response = malloc(BUFFER_SIZE);
    memset(handshake_response, '\0', BUFFER_SIZE);

    int n_read = recv(socket_fd, handshake_response, BUFFER_SIZE, 0);
    return strcmp(handshake_response, "enc_server@") == 0;
}

void get_ciphertext_from_server(int socket_fd, struct Args args)
{
    char *buffer = (char *) malloc(BUFFER_SIZE);
    memset(buffer, '\0', BUFFER_SIZE);

    int full_string_size = BUFFER_SIZE;
    char *full_recd_string = (char *) malloc(full_string_size);
    memset(full_recd_string, '\0', full_string_size);

    int n_read = 0;
    int total_n_read = 0;
    int stop_idx;
    do
    {
        n_read = recv(socket_fd, buffer, BUFFER_SIZE, 0);
        if (n_read < 0)
            fprintf(stderr, "Error: failed to read from socket\n");

        total_n_read += n_read;

        if (total_n_read % full_string_size == 0)
        {
            full_string_size *= 2;
            full_recd_string = (char *) realloc(full_recd_string, full_string_size);
        }
        
        strcat(full_recd_string, buffer);
        memset(buffer, '\0', BUFFER_SIZE);

        find_stop_index(full_recd_string, &stop_idx);
    } while (stop_idx == -1);

    args.ciphertext = (char *) malloc(stop_idx + 1);
    memset(args.ciphertext, '\0', stop_idx + 1);
    strncpy(args.ciphertext, full_recd_string, stop_idx);

    fprintf(stdout, "%s\n", args.ciphertext);
}