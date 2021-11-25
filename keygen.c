/*
Usage: keygen <key_length>
Creates a key file of specified length

    - Characters in the file are any of 27 allowed characters (A-Z and space), generated using rand()
    - A newline character is appended to the file (so length is actually key_length + 1)
    - Outputs any errors to stderr
    - Outputs to stdout
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void error_and_exit(char []);
void generate_key(char *, int);

int main(int argc, char *argv[])
{
    if (argc != 2)
        error_and_exit("Usage: keygen $keylength");
    
    int keylength = atoi(argv[1]);
    if (keylength == 0)
        error_and_exit("Error: key legth argument must be an integer larger than 0\n");
    
    char *key = (char *) malloc(sizeof(char) * (keylength + 2));
    memset(key, '\0', sizeof(char) * (keylength + 2));
    generate_key(key, keylength);

    fprintf(stdout, "%s", key);
    return EXIT_SUCCESS;
}

void error_and_exit(char err[])
{
    fprintf(stderr, "%s\n", err);
    exit(EXIT_FAILURE);
}

void generate_key(char *key, int keylength)
{
    time_t t;
    srand((unsigned) time(&t));

    for (int i = 0; i < keylength; i++)
    {
        int r = rand() % 27;
        if (r == 0)
            key[i] = ' ';
        else
            key[i] = r + 64;
    }
    key[keylength] = '\n';
}