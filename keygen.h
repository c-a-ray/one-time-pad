/**
 * @file keygen.h
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Definitions for keygen.c
 */


#ifndef KEYGEN
#define KEYGEN

/**
 * Converts key length to integer and verifies that it is a valid length
 * 
 * @param  argc the number of command-line arguments given 
 * @param  argv the given command-line arguments
 * 
 * @return true if valid key length was specified, else false
 */
int get_key_length(int, char **);

/**
 * Generates a key of specified length. 
 * Each character is a pseudo-random character.
 * Characters used are A-Z and space.
 * 
 * @param  key string to store key in
 * @param  key_length length of key to generate
 */
void generate_key(char *, int);

#endif