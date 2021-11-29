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

/**
 * Counts the number of digits in an integer
 * 
 * @param  n integer to count digits of
 * 
 * @return number of digits in n
 */
int count_digits(int);

/**
 * If the last character in the provided string is a newline,
 * replaces it with a NULL character.
 * 
 * @param  msg the message to perform character replacement on
 */
void replace_newline(char *);

/**
 * Checks that every character in specified message is valid.
 * Valid characters include A-Z and space. If an invalid character
 * is encountered, the invalid character is returned.
 * 
 * @param  msg the message to validate
 * 
 * @return the first-encountered invalid character if one is found;
 *         0 if no invalid characters were found
 */
char validate_message(char *);

/**
 * Searches through the specified string for the stop character ('@').
 * If the stop character is found, it's index is recorded in stop_idx.
 * If the stop character is not found, it is set to -1.
 * 
 * @param  msg the message to search
 * @param  stop_idx value to hold index of stop character
 */
void find_stop_index(char *, int *);

/**
 * Searches through the specified string for two instances of the stop character ('@').
 * If the first stop character is found, its index is recorded in stop_idx_1.
 * If the second stop character is found, its index is recorded in stop_idx_2.
 * If either stop character is not found, it is set to -1.
 * 
 * @param  msg the message to search
 * @param  stop_idx_1 value to hold index of first stop character
 * @param  stop_idx_2 value to hold index of second stop character
 */
void find_stop_indices(const char *, int *, int *);

/**
 * Performs a modulus operation. C's % operator does not produce
 * the desired behavior with negative numbers, so this function
 * is required for decryption.
 * 
 * @param  a first number in modulus operation
 * @param  b second number in modulus operation
 * 
 * @return (a % b) if (a % b >= 0), else (a % b + b)
 */
int mod(int, int);

#endif