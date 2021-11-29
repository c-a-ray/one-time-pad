/**
 * @file enc_server.c
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Definitions for dec_server.c
 */

#ifndef DEC_SERVER
#define DEC_SERVER

// Object to hold ciphertext, key, and plaintext
struct Args 
{
    char *ciphertext;
    char *key;
    char *plaintext;
};

/**
 * Verifies that specified port is valid and returns it as an integer
 * 
 * @param  argc the number of command-line arguments given 
 * @param  argv the given command-line arguments
 * 
 * @return true if valid port was specified, else false
 */
int get_port(int, char **);

/**
 * Handler for SIGCHLD signal.
 * Performs a non-blocking wait for any child process.
 * When a child process terminates, decrements the number of connections.
 * 
 * @param  signo required but not used
 */
void handle_SIGCHLD(int);

/**
 * Setup signal handler for SIGCHLD; set handler to handle_SIGCHLD()
 * 
 * @return true if successful; false if error is encountered
 */
bool catch_SIGCHLD(void);

/**
 * Handles a single connection. Performs handshake to verify that conneciton
 * is to dec_client, reads ciphertext and key from socket, decrypts ciphertext,
 * and writes plaintext to stdout.
 * 
 * @param  socket_fd file descriptor for connected socket
 */
void handle_connection(int);

/**
 * Verifies that connection is to dec_client
 * 
 * @param  socket_fd file descriptor for connected socket
 * 
 * @return true if connection is to dec_client, else false
 */
bool perform_handshake(int);

/**
 * Decrypts ciphertext using key via one-time-pad, storing plaintext in args
 * 
 * @param  args object holding ciphertext, key, and plaintext
 */
void decrypt(struct Args);

#endif