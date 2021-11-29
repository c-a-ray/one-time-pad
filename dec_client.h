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

#endif