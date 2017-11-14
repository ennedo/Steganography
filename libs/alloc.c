/************************************************************************************
 * @file alloc.c
 * @author Erias Diego, Pisanello Antonio, Rmiza Hassine
 * @date 1 Nov 2017
 * @brief Custom malloc and calloc
 *
 * Does a normal malloc and calloc, but after the memory allocation, it directly
 * checks if it worked
 ***********************************************************************************/
#include "alloc.h"

/***********************************************************
 * Regular malloc but with fail checking
 * @param bytes size of the memory block in bytes
 * @return a pointer to the allocated memory
 ***********************************************************/
void* my_malloc(size_t bytes) {
    void* ptr = malloc(bytes);
    if(ptr == NULL) {
		fprintf(stderr, "TEXT TOO LONG FOR THIS IMAGE\nExiting now...\n");
		exit(EXIT_FAILURE);
    }else{
        return ptr;
    }
}

/***********************************************************
 * Regular calloc but with fail checking
 * @param n number of elements to be allocated
 * @param s size of elements
 * @return a pointer to the allocated memory
 ***********************************************************/
void* my_calloc(size_t n, size_t s) {
    void* ptr = calloc(n, s);
    if(ptr == NULL) {
		fprintf(stderr, "TEXT TOO LONG FOR THIS IMAGE\nExiting now...\n");
		exit(EXIT_FAILURE);
    }else{
        return ptr;
    }
}