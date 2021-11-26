/*
Usage: enc_client <plaintext> <key> <port>
Connects to enc_server to encrypt text

    - The plaintext argument is the name of a file containing the text to encrypt in the current directory
    - The key argument is the name of a file containing a key in the current directory
    - The port parameter is the port that enc_client should try to connect to enc_server on

    - Run validation and output appropriate error to stderr. Check:
        - There are no invalid characters in the plaintext file
        - There are no invalid characters in the key file
        - The key file has at least as many characters as the plaintext file
    - Connects to enc_server
        - If trying to connect to dec_server, reject and report rejection to stderr
        - If can't connect to enc_server, report to stderr with attempted port, set exit value to 2, and terminate
    - Passes key and plaintext to enc_server
    - When enc_client receives ciphertext from enc_server, outputs to stdout
    - Upon successfully running, sets exit value to 0 and exits
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netdb.h>
#include <stdbool.h>

#define LOCALHOST "LOCALHOST"
#define BUFFER_SIZE 256

void error_and_exit(char []);
void setup_socket_addr(struct sockaddr_in *, int);
void validate_send_values(char *, char *);
int count_digits(int);
void send_string(char *, int);
int find_stop_index(char *, int *);

struct Args {
    char *plaintext_filename;
    char *key_filename;
    int port;
};

int main(int argc, char *argv[])
{
    if (argc < 4)
        error_and_exit("Usage: enc_client $plaintext $key $port");

    struct Args args = {
        plaintext_filename: argv[1],
        key_filename: argv[2],
        port: atoi(argv[3]),
    };

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (socket_fd < 0)
        error_and_exit("Error: failed to open socket");

    struct sockaddr_in server_addr;
    setup_socket_addr(&server_addr, args.port);

    if ( connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) )
        error_and_exit("Error: failed to connect to enc_server");

    FILE *plaintext_file = fopen(args.plaintext_filename, "rb");
    if (plaintext_file == 0)
        error_and_exit("Error: failed to open plaintext file");
    fseek(plaintext_file, 0, SEEK_END);
    long f_size = ftell(plaintext_file);
    fseek(plaintext_file, 0, SEEK_SET);
    char *plaintext_string = (char *) malloc(sizeof(char) * (f_size + 1));
    fread(plaintext_string, f_size, 1, plaintext_file);
    fclose(plaintext_file);

    FILE *key_file = fopen(args.key_filename, "rb");
    if (key_file == 0)
        error_and_exit("Error: failed to open plaintext file");
    fseek(key_file, 0, SEEK_END);
    f_size = ftell(key_file);
    fseek(key_file, 0, SEEK_SET);
    char *key_string = (char *) malloc(sizeof(char) * (f_size + 1));
    fread(key_string, f_size, 1, plaintext_file);
    fclose(key_file);

    validate_send_values(plaintext_string, key_string);

    send_string(plaintext_string, socket_fd);
    send_string(key_string, socket_fd);

    // char *buffer = (char *) malloc(BUFFER_SIZE);
    // memset(buffer, '\0', BUFFER_SIZE);

    // int n_read = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0);

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

    char *ciphertext = (char *) malloc(stop_idx + 1);
    memset(ciphertext, '\0', stop_idx + 1);
    strncpy(ciphertext, full_recd_string, stop_idx);

    fprintf(stdout, "%s\n", ciphertext);

    return EXIT_SUCCESS;
}

void error_and_exit(char err[])
{
    fprintf(stderr, "%s\n", err);
    exit(EXIT_FAILURE);
}

void setup_socket_addr(struct sockaddr_in *address, int port)
{
    memset( (char*) address, '\0', sizeof(*address) );
    
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    struct hostent *host_info = gethostbyname(LOCALHOST);
    if (host_info == NULL)
        error_and_exit("Error: host not found");

    memcpy( (char *) &address->sin_addr.s_addr, host_info->h_addr_list[0], host_info->h_length );
}

void validate_send_values(char *plaintext_string, char *key_string)
{
    int plaintext_len = strlen(plaintext_string);
    int key_len = strlen(key_string);

    if (plaintext_string[plaintext_len - 1] == '\n')
        plaintext_string[plaintext_len - 1] = '\0';
    
    if(key_string[key_len - 1] == '\n')
        key_string[key_len - 1] = '\0';

    if (key_len < plaintext_len)
        error_and_exit("Error: key is shorter than text to encrypt");

    for (int i = 0; i < plaintext_len; i++)
    {
        char ch = plaintext_string[i];
        if ((ch < 'A' && ch != ' ') || (ch > 'Z'))
            error_and_exit("Error: invalid character in plaintext file");
    }
}

int count_digits(int n)
{
    int count = 0;
    while (n > 0)
    {
        n = n / 10;
        count++;
    }
    return count;
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

int find_stop_index(char *message, int *stop_idx)
{
    *stop_idx = -1;
    for (int i = 0; i < strlen(message); i++)
    {
        if (message[i] == '@')
        {
            *stop_idx = i;
            break;
        }
    }
}