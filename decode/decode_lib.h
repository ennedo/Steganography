/************************************************************************************
 * @file encode_lib.h
 * @author Erias Diego, Pisanello Antonio, Rmiza Hassine
 * @date 1 nov 2017
 * @brief Routines used by the main decode file
 ***********************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "../libs/ppm.h"

#define BYTES_HEADER_CHAR 32

int add_pow_2(uint8_t *ptr, int exp);
int get_nb_char_img(img_t *img);


