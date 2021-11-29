/**
 * @file util.h
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 5
 * 
 * Definitions for util.c
 */

#ifndef UTIL
#define UTIL

int count_digits(int);
void replace_newline(char *);
char validate_message(char *);
void find_stop_index(char *, int *);
void find_stop_indices(const char *, int *, int *);
int mod(int, int);

#endif