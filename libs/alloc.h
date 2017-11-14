/************************************************************************************
 * @file alloc.h
 * @author Erias Diego, Pisanello Antonio, Rmiza Hassine
 * @date 1 Nov 2017
 * @brief Custom malloc and calloc
 ***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void* my_malloc(size_t bytes);
void* my_calloc(size_t n, size_t s);