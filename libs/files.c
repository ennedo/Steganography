/************************************************************************************
 * @file files.h
 * @author Erias Diego, Pisanello Antonio, Rmiza Hassine
 * @date 1 Nov 2017
 * @brief Routines to treats with text files
 ***********************************************************************************/
#include "files.h"

/***********************************************************
 * Open a (text) file with it's path
 * @param filename string containing the name of the file
 * @param mode specify the file access mode 
 * @return a pointer to the stream of the file
 ***********************************************************/
FILE *open_file(char* filename, char *mode){
	FILE * file = fopen(filename,mode);
    // If there is an error and the pointer is NULL
	if(!file){
		fprintf(stderr, "FILE %s NOT FOUND OR CANNOT BE OPENED\n", filename);
		exit(EXIT_FAILURE);
	}

	return file;
}

/***********************************************************
 * Get the size of a file
 * @param filename string containing the name of the file
 * @return the file size
 ***********************************************************/
off_t fsize(const char *filename){
    // Get the stats of a file (the size is in the stats)
    struct stat st;
    if (!stat(filename, &st))
        return st.st_size;

    // If error
    fprintf(stderr, "CANNOT DETERMINATE SIZE OF %s\n", filename);
    exit(EXIT_FAILURE);
}

/***********************************************************
 * Get the string of a text file
 * @param filename string containing the name of the file
 * @param nb_char the number of chars in the text file
 * @param s a pointer to the final string of the text file
 ***********************************************************/
void file_to_str(char* filename, int nb_char , char **s){
	FILE *fp = open_file(filename, "r");

	*s = my_calloc(nb_char + 1, sizeof(char));
	fread(*s, 1, nb_char, fp);

  	fclose (fp);
}