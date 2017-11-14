/************************************************************************************
 * @file encode_lib.c
 * @author Erias Diego, Pisanello Antonio, Rmiza Hassine
 * @date 1 nov 2017
 * @brief Decode a hidden text encoded into a ppm image
 *
 * This program will decode a text hidden in a ppm image (argument 1).
 * The text is hidden in the lowest bit of R, G or B.
 * It will decode this text in multi-threading (argument 2)
 ***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include "decode_lib.h"
#include "../libs/alloc.h"

#define BITS_PER_CHAR 7
#define FIRST_PIXEL 11
#define NB_ARG 2

/***********************************************************
 * Store where the threads begins
 * @param initial_indice Pixel of the image
 * @param initial_pos_rgb R, G or B (0, 1 or 2) in the pixel
 ***********************************************************/
typedef struct limit_threads_st {
	int initial_indice;
	int initial_pos_rgb;
} limit_threads_t;

/***********************************************************
 * Store the arguments for the threads
 * @param limit see the struct limit_threads_t
 * @param char_interval number of char to decode
 * @param img a pointer to the image to read
 ***********************************************************/
typedef struct param_st {
	limit_threads_t limit;
	int char_interval;
	img_t *img;
} param_t;

/***********************************************************
 * To know the limits of a thread
 * @param min first char position of the cut text
 * @return where the threads begins (pixel position)
 ***********************************************************/
limit_threads_t get_limits(int min){
	limit_threads_t ret;
	
	ret.initial_indice = floor((float)(min * BITS_PER_CHAR) / sizeof(pixel_t)) + FIRST_PIXEL;
	ret.initial_pos_rgb = (min * BITS_PER_CHAR) % sizeof(pixel_t);

	return ret;
}

/***********************************************************
 * Threads doing the decoing
 * @param param see the struct param_t
 * @return return one part of the decoded text
 ***********************************************************/
void *thread(void *param){
    // Get arguments
    param_t *p = (param_t *)param;
    int initial_ind = p->limit.initial_indice;
    int initial_pos = p->limit.initial_pos_rgb;
    img_t *img = p->img;
    // Position the pointer to the first pixel (on R, G or B) we want to decode
	uint8_t *ptr = &img->raw[initial_ind].r + initial_pos;
    
    // Intialise return string pointer
    char *str_return = my_calloc(p->char_interval+1, sizeof(char));
    
    // This loop go through each pixel we decode
    for (int i = 0; i < p->char_interval; i++){
        // Initialize current char to 0
    	char tmp_char = 0;
        // For each char it must decode 7 lowest bits
    	for (int indice = BITS_PER_CHAR-1; indice >= 0; indice--){
            // If the lowest bit is 1, the right pow of 2 is added
    		tmp_char += add_pow_2(ptr, indice);
            // Reposition the pixel to the next R, G or B
    		ptr++;
    	}
        // 1 decoded char is added into the string of return
		str_return[i] = tmp_char;
    }
    return str_return;
}

/***********************************************************
 * Display the program's syntaxe.
 * @param argv program's command line arguments
 ***********************************************************/
void usage(char **argv){
	fprintf(stderr, "\nNumber of arguments is invalid\n\n"\
        "usage: %s image thread_count\n"\
		"       where image is a PPM file containing an encoded secret message\n"\
		"       and thread_count the number of threads to use.\n", basename(argv[0]));
	exit(EXIT_FAILURE);
}

/****************************************************
 * Program entry point.
 * @param argc command line argument count
 * @param argv program's command line arguments
 ****************************************************/
int main(int argc, char **argv){
	// Parse command line
    if(argc != NB_ARG + 1)
		usage(argv);
	char *input=argv[1];
	int nb_threads = atoi(argv[2]);
	
	img_t *img = load_ppm(input);
	int nb_char = get_nb_char_img(img);
    float interval;
	void *part_str_decoded;
	char *text_decoded = my_calloc(nb_char + 1, sizeof(char));
    
    // Check if there is a correct number of threads
    if(nb_threads <= 0){
        fprintf(stderr,"NUMBERS OF THREADS MUST BE GREATER THAN ZERO\nExiting now...\n");
		exit(EXIT_FAILURE);
    }
    
    // Check if there is more threads thans char in the text and allocate memory
	if(nb_threads > (int)nb_char)
        nb_threads = nb_char;
	param_t *threads_param = my_malloc(nb_threads * sizeof(param_t));
	pthread_t *threads = my_malloc(nb_threads * sizeof(pthread_t));
    
    // Get the general (float) interval of each decoded texts part
    interval = (float)nb_char / (float)nb_threads;
    
    // Threads launching loop
	for (int i = 0; i < nb_threads; i++){
        // Get the "real" interval and limits
		int min = round(interval * i) + 0;
		int char_in_interval = round(interval * (i + 1)) - min;
        
        // Assign the threads arguments
        threads_param[i].limit = get_limits(min);
        threads_param[i].char_interval = char_in_interval;
        threads_param[i].img = img;

        // Create thread and check for fail
        if (pthread_create(&threads[i], NULL, thread, &threads_param[i]) != 0){
            fprintf(stderr, "pthread_create failed!\n");
            exit(EXIT_FAILURE);
        }
	}
    
    // Threads "waiting" loop
	for (int i = 0; i < nb_threads; i++){
        pthread_join(threads[i], &part_str_decoded);
        
        // Add the part of the decoded text into the final result
        strcat(text_decoded, part_str_decoded);
        
        free(part_str_decoded);
    }
    
    printf("\n%u threads were used\n\n", nb_threads);
    
    // Print the text decoded
    printf("---------- TEXT DECODED ----------\n\n%s"\
       "\n\n---------- TEXT DECODED ----------\n\n", text_decoded);
    
    free_img(img);
    free(text_decoded);
    free(threads);
    free(threads_param);
       
	return EXIT_SUCCESS;
}
