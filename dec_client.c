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

#define LOCALHOST "LOCALHOST"
#define BUFFER_SIZE 4096

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

void error_and_exit(char []);
void setup_socket_addr(struct sockaddr_in *, int);
void validate_send_values(char *, char *);
int connect_to_server(int);
int count_digits(int);
void send_string(char *, int);
int find_stop_index(char *, int *);
void get_plaintext_from_server(int, struct Args);

int main(int argc, char *argv[])
{
    // Get command line arguments
    if (argc < 4)
        error_and_exit("Usage: dec_client $ciphertext $key $port");

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
        error_and_exit("Error: failed to open ciphertext file");

    fseek(fp_ciphertext, 0, SEEK_END);
    long f_size = ftell(fp_ciphertext);
    fseek(fp_ciphertext, 0, SEEK_SET);

    args.ciphertext = (char *) malloc(f_size + 1);
    fread(args.ciphertext, f_size, 1, fp_ciphertext);
    fclose(fp_ciphertext);

    // Get key from file
    FILE *fp_key = fopen(cfg.key_filename, "rb");
    if (fp_key == 0)
        error_and_exit("Error: failed to open key file");

    fseek(fp_key, 0, SEEK_END);
    f_size = ftell(fp_key);
    fseek(fp_key, 0, SEEK_SET);

    args.key = (char *) malloc(f_size + 1);
    fread(args.key, f_size, 1, fp_key);
    fclose(fp_key);

    // Validate ciphertext and key; replace newline with NULL character
    validate_send_values(args.ciphertext, args.key);

    // Connect to dec_server
    int socket_fd = connect_to_server(cfg.port);
    
    // Send handshake
    send_string("dec_client", socket_fd);
    
    char *handshake_response = malloc(BUFFER_SIZE);
    memset(handshake_response, '\0', BUFFER_SIZE);
    int n_read = recv(socket_fd, handshake_response, BUFFER_SIZE, 0);

    if (strcmp(handshake_response, "dec_server@") != 0)
        error_and_exit("Handshake failed. Invalid connection.\n");

    // Send ciphertext and key to dec_server
    send_string(args.ciphertext, socket_fd);
    send_string(args.key, socket_fd);

    // Get plaintext from server and write to stdout
    get_plaintext_from_server(socket_fd, args);

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

void validate_send_values(char *ciphertext, char *key)
{
    int ciphertext_len = strlen(ciphertext);
    int key_len = strlen(key);

    if (ciphertext[ciphertext_len - 1] == '\n')
        ciphertext[ciphertext_len - 1] = '\0';
    
    if(key[key_len - 1] == '\n')
        key[key_len - 1] = '\0';

    if (key_len < ciphertext_len)
        error_and_exit("Error: key is shorter than text to decrypt");

    for (int i = 0; i < ciphertext_len; i++)
    {
        char ch = ciphertext[i];
        if (ch == 0)
            continue;
        if ((ch < 'A' && ch != ' ') || (ch > 'Z'))
            error_and_exit("Error: invalid character in plaintext file");
    }
}

int connect_to_server(int port)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (socket_fd < 0)
        error_and_exit("Error: failed to open socket");

    struct sockaddr_in server_addr;
    setup_socket_addr(&server_addr, port);

    if ( connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) )
        error_and_exit("Error: failed to connect to dec_server");   

    return socket_fd; 
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