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
    if (argc != 2)
    {
        fprintf(stderr, "Error: key length must be specified\n");
        fprintf(stderr, "Usage: keygen $keylength\n");
        return EXIT_FAILURE;
    }
    
    int keylength = atoi(argv[1]);
    if (keylength == 0)
    {
        fprintf(stderr, "Error: key length argument must be an integer larger than 0\n");
        return EXIT_FAILURE;
    }
    
    char *key = (char *) malloc(keylength + 2);
    memset(key, '\0', keylength + 2);
    generate_key(key, keylength);

    fprintf(stdout, "%s", key);
    return EXIT_SUCCESS;
}

void generate_key(char *key, int keylength)
{
    time_t t;
    srand((unsigned) time(&t));
    int r_num;

    for (int i = 0; i < keylength; i++)
    {
        r_num = rand() % 27;
        key[i] = r_num == 0 ? ' ' : r_num + 64;
    }
    key[keylength] = '\n';
}