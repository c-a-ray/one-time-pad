/**
 * @file enc_client.h
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Definitions for enc_client.c
 */

#ifndef ENC_CLIENT
#define ENC_CLIENT

// Object to store arguments given by user
struct Config 
{
    char *plaintext_filename;
    char *key_filename;
    int port;
};

// Object to store plaintext, key and ciphertext
struct Args 
{
    char *plaintext;
    char *key;
    char *ciphertext;
};

/**
 * Verifies that established connection is to enc_server.
 * Identifies self as enc_client and waits for enc_server to identify itself.
 * If connected server is enc_server, function returns true, otherwise false.
 * 
 * @param  socket_fd file descriptor for connected socket
 * 
 * @return true if connection is to enc_server, else false
 */
bool perform_handshake(int);

/**
 * Reads ciphertext from enc_server, then writes ciphertext to stdout.
 * 
 * @param  socket_fd file descriptor for connected socket
 * @param  args object to store plaintext, key, and ciphertext
 */
void get_ciphertext_from_server(int, struct Args);

#endif