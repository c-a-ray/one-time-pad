/**
 * @file util.c
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Contains shared utility functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

int count_digits(int n)
{
    // Digit counter
    int count = 0;

    // Iterate until n is 0
    while (n > 0)
    {
        // Divide n by 10 and increment digit counter
        n = n / 10;
        count++;
    }

    // Return number of digits
    return count;
}

void replace_newline(char *msg)
{
    int msg_len = strlen(msg);

    // If the last character is a newline, change it to '\0'
    if (msg[msg_len - 1] == '\n')
        msg[msg_len - 1] = '\0';
}

char validate_message(char *msg)
{
    // Walk through all characters in message
    for (int i = 0; i < strlen(msg); i++)
    {
        // Get the character at the current index
        char ch = msg[i];

        // If character is 0, skip
        if (ch == 0)
            continue;

        // If character is not valid, return the character
        if ( (ch < 'A' && ch != ' ') || ch > 'Z' )
            return ch;
    }

    // Return 0 to indicate no invalid characters
    return 0;
}

void find_stop_index(char *message, int *stop_idx)
{
    // stop_idx of -1 indicates stop character has not been found
    *stop_idx = -1;

    // Walk through every character in message
    for (int i = 0; i < strlen(message); i++)
    {
        // Check if the character is the stop character
        if (message[i] == '@')
        {
            // If it is, record its index and return
            *stop_idx = i;
            break;
        }
    }
}

void find_stop_indices(const char *buffer, int *stop_idx_1, int *stop_idx_2)
{
    *stop_idx_1 = -1;
    *stop_idx_2 = -1;

    for (int i = 0; i < strlen(buffer); i++)
    {
        if (buffer[i] == '@')
        {
            if (*stop_idx_1 == -1)
                *stop_idx_1 = i;
            else if (*stop_idx_2 == -1)
            {
                *stop_idx_2 = i;
                break;
            }
        }
    }
}

int mod(int a, int b)
{
    // Get the remainder of a / b
    int remainder = a % b;

    // If the remainder is negative, return remainder + b
    // Otherwise return remainder
    return remainder < 0 ? remainder + b : remainder;
}