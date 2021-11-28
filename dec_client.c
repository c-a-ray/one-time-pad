/*
Usage: dec_client <plaintext> <key> <port>
Connects to dec_server to decrypt text
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netdb.h>
#include <stdbool.h>

#include "socket_io.h"
#include "util.h"

struct Config 
{
    char *ciphertext_filename;
    char *key_filename;
    int port;
};

struct Args 
{
    char *ciphertext;
    char *key;
    char *plaintext;
};

bool perform_handshake(int);
void get_plaintext_from_server(int, struct Args);

int main(int argc, char *argv[])
{
    // Get command line arguments
    if (argc < 4)
    {
        fprintf(stderr, "Error: missing %d arguments\n", 4 - argc);
        return EXIT_FAILURE;
    }

    struct Config cfg = {
        ciphertext_filename: argv[1],
        key_filename: argv[2],
        port: atoi(argv[3]),
    };

    // Extract values from specified files
    struct Args args;

    // Get ciphertext from file
    FILE *fp_ciphertext = fopen(cfg.ciphertext_filename, "rb");
        
    if (fp_ciphertext == 0)
    {
        fprintf(stderr, "Error: failed to open ciphertext file \"%s\"\n", cfg.ciphertext_filename);
        return EXIT_FAILURE;
    }

    fseek(fp_ciphertext, 0, SEEK_END);
    long f_size = ftell(fp_ciphertext);
    fseek(fp_ciphertext, 0, SEEK_SET);

    args.ciphertext = (char *) malloc(f_size + 1);
    fread(args.ciphertext, f_size, 1, fp_ciphertext);
    fclose(fp_ciphertext);

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

    // Replace newline with null character in ciphertext and check for invalid characters
    replace_newline(args.ciphertext);
    char invalid_char = validate_message(args.ciphertext);
    if (invalid_char != 0)
    {
        fprintf(stderr, "Error: invalid character in ciphertext file: %c\n", invalid_char);
        return EXIT_FAILURE;
    }

    // Replace newline with null character in key
    replace_newline(args.key);

    // Make sure key is long enough
    if (strlen(args.ciphertext) > strlen(args.key))
    {
        fprintf(stderr, "Error: ciphertext is longer than key\n");
        fprintf(stderr, "Ciphertext length: %d\tKey length: %d\n", strlen(args.ciphertext), strlen(args.key));
        return EXIT_FAILURE;
    }

    // Connect to dec_server
    int socket_fd = connect_to_server(cfg.port);
    
    // Verify that dec_client is connecting to dec_server
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
    send_string("dec_client", socket_fd);
    
    char *handshake_response = malloc(BUFFER_SIZE);
    memset(handshake_response, '\0', BUFFER_SIZE);

    int n_read = recv(socket_fd, handshake_response, BUFFER_SIZE, 0);
    
    if ( strcmp(handshake_response, "enc_server@") == 0 )
    {
        fprintf(stderr, "Error: connection refused: dec_client cannot connect to enc_server\n");
        return false;
    }
    if ( !strcmp(handshake_response, "dec_server@") == 0 )
    {
        fprintf(stderr, "Error: connection refused: unknown server: %s\n", handshake_response);
        return false;
    }

    return true;
}

void get_plaintext_from_server(int socket_fd, struct Args args)
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

    args.plaintext = (char *) malloc(stop_idx + 1);
    memset(args.plaintext, '\0', stop_idx + 1);
    strncpy(args.plaintext, full_recd_string, stop_idx);

    fprintf(stdout, "%s\n", args.plaintext);
}