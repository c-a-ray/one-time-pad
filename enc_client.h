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

struct Config 
{
    char *plaintext_filename;
    char *key_filename;
    int port;
};

struct Args 
{
    char *plaintext;
    char *key;
    char *ciphertext;
};

bool perform_handshake(int);
void get_ciphertext_from_server(int, struct Args);

#endif