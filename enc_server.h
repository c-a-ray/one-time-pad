/**
 * @file enc_server.h
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Definitions for enc_server.c
 */

#ifndef ENC_SERVER
#define ENC_SERVER

struct Args 
{
    char *plaintext;
    char *key;
    char *ciphertext;
};

int get_port(int, char **);
void handle_SIGCHLD(int);
bool catch_SIGCHLD(void);
void handle_connection(int);
bool perform_handshake(int);
void encrypt(struct Args);

#endif