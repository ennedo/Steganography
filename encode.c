#include <sys/stat.h>
#include "encode_lib.h"
#include "ppm.h"
#include <string.h>
#include <errno.h>
#include <pthread.h>
//hello
img_t *img;

typedef struct limit_threads_st {
	int initial_indice;
	int initial_pos_rgb;
	int final_indice;
	int final_pos_rgb;
} limit_threads_t;

typedef struct thread_st {
	limit_threads_t limit;
	char *array;
} thread_t;

FILE * open_file(char* filename, char *mode){
	FILE * file = fopen(filename,mode);
	if(!file)
	{
		printf("file not found: %s\n", filename);
		exit(0);
	}

	return file;
}

char *int2bin(int a, char *buffer, int buf_size) {
    buffer += (buf_size - 1);

    for (int i = 31; i >= 0; i--) {
        *buffer-- = (a & 1) + '0';

        a >>= 1;
    }

    return buffer;
}

off_t fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    fprintf(stderr, "Cannot determine size of %s: %s\n",
            filename, strerror(errno));

    return -1;
}

void file_to_str(char* filename, int nb_char , char **s){

	FILE *fp = open_file(filename, "r");

	*s = calloc(nb_char + 1, sizeof(char));
	fread(*s, 1, nb_char, fp);

  	fclose (fp);
}


int max_char_encode(img_t *img){
	return floor((img->height * img->width - 11) * 3 / BITS_PER_CHAR);
}

limit_threads_t get_limits(int min, int max)
{
	limit_threads_t ret;

	ret.initial_indice = floor((float)min/3);
	ret.final_indice = floor((float)max/3);

	ret.initial_pos_rgb = min % 3;
	ret.final_pos_rgb = max % 3;

	//printf("initial_indice : %d reste min %d / final_indice : %d / reste max %d \n", ret.initial_indice, ret.initial_pos_rgb, ret.final_indice, ret.final_pos_rgb);

	return ret;
}

int main(int argc, char **argv){
	img = load_ppm("img.ppm");
	int nb_threads = 50;
	pthread_t *threads = malloc(sizeof(pthread_t) * nb_threads);
	thread_t *threads_param = malloc(sizeof(pthread_t) * nb_threads);
	char* text;
	int max_char = max_char_encode(img);
	printf("MaxChar : %d\n", max_char);
	int nb_char = fsize("text.txt");
	int nb_char_encoded = nb_char * BITS_PER_CHAR;
	char* text_encoded;
	float interval;
	limit_threads_t limit;
	char *nb_char_header = calloc(32+1, sizeof(char));

	if (nb_char > max_char){
		printf("Fichier texte trop long pour l'image\n");
		exit(0);
	}
		
	int2bin(nb_char, nb_char_header, 32);
	// nb_char_header[33] = 0;
	printf("------ nb_char_header %s\n", nb_char_header);

	file_to_str("text.txt", nb_char, &text);
	encode_str(text,nb_char,&text_encoded);
	printf("%s", text_encoded);
	printf("\n\nnb char * 7 -> %u\nstrlen -> %u\n",nb_char*7,strlen(text_encoded));

	interval = (float)nb_char_encoded / (float)nb_threads;
	printf("%f\n\n\n",interval);

	

	for (int i = 0; i < nb_threads; i++){
		int min = round(interval * i) + 0;
		int max = round(interval * (i + 1)) - 1;
		int char_in_interval = max + 1 - min;

		char *test = calloc(char_in_interval + 1, sizeof(char));

		if (char_in_interval > 0){

			limit = get_limits(min,max);

			memcpy(test, text_encoded + min, char_in_interval);
			test[char_in_interval] = 0;
			printf("%s", test);
			printf("\nInterval: %u, Strlen: %u", char_in_interval, strlen(test));
		}
		printf("\nMin: %u / Max: %u / Interval: %u\n\n", min, max, char_in_interval);
	}
	return 0;
}
