/************************************************************************************
 * @file encode_lib.h
 * @author Erias Diego, Pisanello Antonio, Rmiza Hassine
 * @date 1 nov 2017
 * @brief Routines used by the main decode file
 ***********************************************************************************/
#include "decode_lib.h"

/***********************************************************
 * Get a R, B or B, read the lowest bit
 * @param rgb a pointer to the component we are decoding
 * @param exp exposant of pow 2
 * @return return the pow of 2 if lowest bit is 1
 ***********************************************************/
int add_pow_2(uint8_t *rgb, int exp){
	return (*rgb % 2) * pow(2, exp);
}

/***********************************************************
 * Get number of char encoded in the image (32 lowest bits)
 * @param img a pointer to the image
 * @return return the number of char
 ***********************************************************/
int get_nb_char_img(img_t *img){
    // Point into the first Pixel
	uint8_t *rgb = &img->raw[0].r;
	int nb_char=0, exp=0;
    
    // Starts with exp = 1 and ends with exp = 31
	for (int i = BYTES_HEADER_CHAR-1; i >= 0; i--){
        // If the lowest bit is 1, the right pow of 2 is added
		nb_char += add_pow_2(rgb + i, exp);
		exp++;
	}
	return nb_char;
}