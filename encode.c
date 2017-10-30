#include <sys/stat.h>
#include "encode_lib.h"
#include "./ppm/ppm.h"
#include <string.h>
#include <errno.h>
#include <pthread.h>

img_t **img;

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

//limit_threads_t get_limits(int temppos, int interval, int min, int max)
limit_threads_t get_limits(int previous_pos[2], int interval, int min, int max)
{
	limit_threads_t ret;

	if (previous_pos[0] != 0)
	{
		if (previous_pos[1] == 2)
		{
			previous_pos[0]++;
			previous_pos[1] = 0;
		}
		else
		{
			previous_pos[1]++;
		}
	}
	

	//printf("temppos : %d / interval : %d\n", temppos, interval);

	ret.initial_pos_rgb = previous_pos[1];
	ret.final_pos_rgb = (previous_pos[0]+interval) % 3;
	ret.initial_indice = previous_pos[0];
	ret.final_indice = ceil((float)previous_pos[0]+(float)interval/3);
	//printf("\n coucou: %f\n",ceil((float)(temppos+interval)/3));
	printf("initial_indice : %d reste min %d / final_indice : %d / reste max %d \n", ret.initial_indice, ret.initial_pos_rgb, ret.final_indice, ret.final_pos_rgb);

	return ret;
}

int main(int argc, char **argv){
	img = load_ppm("./ppm/img.ppm");
	int nb_threads = 50;
	pthread_t *threads = malloc(sizeof(pthread_t) * nb_threads);
	thread_t *threads_param = malloc(sizeof(pthread_t) * nb_threads);
	char* text;
	int max_char = max_char_encode(img);
	int nb_char = fsize("text.txt");
	int nb_char_encoded = nb_char * BITS_PER_CHAR;
	char* text_encoded;
	float interval;
	limit_threads_t limit;

	//thread_t


	if (nb_char > max_char){
		printf("Fichier texte trop long pour l'image\n");
		exit(0);
	}

	file_to_str("text.txt", nb_char, &text);
	encode_str(text,nb_char,&text_encoded);
	printf("%s", text_encoded);
	printf("\n\nnb char * 7 -> %u\nstrlen -> %u\n",nb_char*7,strlen(text_encoded));

	int pos = 0;
	int previous_pos[2] = {0,0};
	interval = (float)nb_char_encoded / (float)nb_threads;
	printf("%f\n\n\n",interval);
	for (int i = 0; i < nb_threads; i++){
		int min = round(interval * i) + 0;
		int max = round(interval * (i + 1)) - 1;
		int char_in_interval = max + 1 - min;


		

		
		char *test = calloc(char_in_interval + 1, sizeof(char));
		if (char_in_interval > 0){
			//limit = get_limits(previous_pos[0]*3+previous_pos[1],char_in_interval,min,max);
			limit = get_limits(previous_pos,char_in_interval,min,max);
			previous_pos[0] = limit.final_indice;
			previous_pos[1] = limit.final_pos_rgb;

			// threads_param[i] = 
			memcpy(test, text_encoded + min, char_in_interval);
			test[char_in_interval] = 0;
			printf("%s", test);
			printf("\nInterval: %u, Strlen: %u", char_in_interval, strlen(test));
		pos++;
		}
		pos += char_in_interval;
		printf("\nMin: %u / Max: %u / Interval: %u\n\n", min, max, char_in_interval);
	}
	return 0;
}
