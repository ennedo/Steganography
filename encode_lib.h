/**
 * @file ppm.h
 * @author Florent Gluck
 * @date 17 Oct 2017
 * @brief Routines to read and write PPM files.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define BITS_PER_CHAR 7

void decode_char(char a, char* b);
