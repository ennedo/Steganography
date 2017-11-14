/************************************************************************************
 * @file files.h
 * @author Erias Diego, Pisanello Antonio, Rmiza Hassine
 * @date 1 Nov 2017
 * @brief Routines to treats with text files
 ***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "alloc.h"

FILE *open_file(char* filename, char *mode);
off_t fsize(const char *filename);
void file_to_str(char* filename, int nb_char , char **s);