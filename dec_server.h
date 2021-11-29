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

struct Args 
{
    char *ciphertext;
    char *key;
    char *plaintext;
};

int get_port(int, char **);
void handle_SIGCHLD(int);
bool catch_SIGCHLD(void);
void handle_connection(int);
bool perform_handshake(int);
void decrypt(struct Args);

#endif