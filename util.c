/**
 * @file util.c
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Contains shared helper functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

int count_digits(int n)
{
    int count = 0;
    while (n > 0)
    {
        n = n / 10;
        count++;
    }
    return count;
}

void replace_newline(char *msg)
{
    int msg_len = strlen(msg);
    if (msg[msg_len - 1] == '\n')
        msg[msg_len - 1] = '\0';
}

char validate_message(char *msg)
{
    for (int i = 0; i < strlen(msg); i++)
    {
        char ch = msg[i];
        if (ch == 0)
            continue;
        if ( (ch < 'A' && ch != ' ') || ch > 'Z' )
            return ch;
    }
    return 0;
}

void find_stop_index(char *message, int *stop_idx)
{
    *stop_idx = -1;
    for (int i = 0; i < strlen(message); i++)
    {
        if (message[i] == '@')
        {
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
    int remainder = a % b;
    return remainder < 0 ? remainder + b : remainder;
}