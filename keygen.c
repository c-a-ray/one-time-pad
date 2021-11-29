/**
 * @file keygen.c
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Creates a key file of specified length and writes it to stdout.
 * Characters include A-Z and space.
 * 
 * Usage: keygen $keylength
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "keygen.h"

int main(int argc, char *argv[])
{
    // Verify key length is valid and convert it to an integer
    int key_length = get_key_length(argc, argv);
    if (key_length == 0)
        return EXIT_FAILURE;
    
    // Generate key of specified length
    char *key = (char *) malloc(key_length + 2);
    memset(key, '\0', key_length + 2);
    generate_key(key, key_length);

    // Write key to stdout
    fprintf(stdout, "%s", key);
    return EXIT_SUCCESS;
}

int get_key_length(int argc, char **argv)
{
    // Verify that a key length was specified
    if (argc != 2)
    {
        fprintf(stderr, "Error: key length must be specified\n");
        fprintf(stderr, "Usage: keygen $keylength\n");
        return 0;
    }

    // Convert key length to integer
    int key_length = atoi(argv[1]);

    // Print error if key length is not valid
    if (key_length == 0)
        fprintf(stderr, "Error: key length argument must be an integer larger than 0\n");

    return key_length;
}

void generate_key(char *key, int key_length)
{
    // Seed pseudo-random number generater
    time_t t;
    srand((unsigned) time(&t));

    // Iterate key_length times
    for (int i = 0; i < key_length; i++)
    {
        // Get a pseudo-random number between 0 and 26
        int r_num = rand() % 27;

        // Convert r_num to character and store in key
        key[i] = r_num == 0 ? ' ' : r_num + 64;
    }

    // Add a newline as the last character
    key[key_length] = '\n';
}