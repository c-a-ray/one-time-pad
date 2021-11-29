/**
 * @file dec_client.h
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Definitions for dec_client.c
 */

#ifndef DEC_CLIENT
#define DEC_CLIENT

// Object to store arguments given by user
struct Config 
{
    char *ciphertext_filename;
    char *key_filename;
    int port;
};

// Object to store ciphertext, key and plaintext
struct Args 
{
    char *ciphertext;
    char *key;
    char *plaintext;
};

/**
 * Verifies that established connection is to dec_server.
 * Identifies self as dec_client and waits for dec_server to identify itself.
 * If connected server is dec_server, function returns true, otherwise false.
 * 
 * @param  socket_fd file descriptor for connected socket
 * 
 * @return true if connection is to dec_server, else false
 */
bool perform_handshake(int);

/**
 * Reads plaintext from dec_server and writes it to stdout
 * 
 * @param  socket_fd file descriptor for connected socket
 * @param  args object to store ciphertext, key, and plaintext
 */
void get_plaintext_from_server(int, struct Args);

#endif