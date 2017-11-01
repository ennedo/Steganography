#include <sys/stat.h>
#include "encode_lib.h"
#include "ppm.h"
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define FIRST_PIXEL 11

typedef struct limit_threads_st {
	int initial_indice;
	int initial_pos_rgb;
} limit_threads_t;

typedef struct param_st {
	int thread_id;
	limit_threads_t limit;
	char *text_cut;
	img_t **img_out;
} param_t;

FILE * open_file(char* filename, char *mode){
	FILE * file = fopen(filename,mode);
	if(!file){
		printf("file not found: %s\n", filename);
		exit(0);
	}

	return file;
}

char *int2bin(int a, char *buffer, int buf_size){
    buffer += (buf_size - 1);
    for (int i = buf_size - 1; i >= 0; i--){
        *buffer-- = (a & 1) + '0';
        a >>= 1;
    }
    return buffer;
}

off_t fsize(const char *filename){
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
	return floor((img->height * img->width - FIRST_PIXEL) * sizeof(pixel_t) / BITS_PER_CHAR);
}

limit_threads_t get_limits(int min){
	limit_threads_t ret;
	
	ret.initial_indice = floor((float)min / sizeof(pixel_t)) + FIRST_PIXEL;
	ret.initial_pos_rgb = min % sizeof(pixel_t);

	return ret;
}


void write_nb_char_in_img(char *nb_char, img_t **img_out){
	printf("%s\n", nb_char);

	uint8_t rgb = 0;
	uint8_t *test_rgb = &(*img_out)->raw[0].r;

	for (int i = 0; i < strlen(nb_char); i++)
		*(test_rgb+i) = encode_char(*(test_rgb+i), nb_char[i]);
}

void show_img(img_t *img, int nb_pixel){
    int max_pixel = img->height * img->width;
    uint8_t *ptr = &img->raw[0].r;
    printf("ptr %d",*ptr);
    if (nb_pixel > max_pixel)
        nb_pixel = max_pixel;
    char* temp = calloc(9, sizeof(char));
    printf("\n\n\n--------- image ---------\n");
    for(int i = 0; i < nb_pixel; i++){
        for(int j = 0; j < sizeof(pixel_t); j++){
            int2bin(*(ptr+i*sizeof(pixel_t)+j), temp, 8);
            printf("   %3d: %3d %s\n",i,*(ptr+i*sizeof(pixel_t)+j),temp);
        }
        printf("\n");
    }   
    printf("--------- image ---------\n\n\n");
}

void *thread(void *para){
    param_t *p = (param_t *)para;

    printf("hello from thread : %d\n", p->thread_id);

    int initial_ind = p->limit.initial_indice;
    int initial_pos = p->limit.initial_pos_rgb;
    img_t **img_out = p->img_out;

 	printf("initial_indice for thread->%d : %d\n", p->thread_id, initial_ind);
   
    uint8_t *test_rgb = &(*img_out)->raw[initial_ind].r + initial_pos;
    
    printf("RGB for threads i=%d : %d\n", p->thread_id, *test_rgb);

    for (int i = 0; i < strlen(p->text_cut); i++)
		*(test_rgb+i) = encode_char(*(test_rgb+i), p->text_cut[i]);
}

int main(int argc, char **argv){
	img_t *img, *img_out;
	img = load_ppm("img.ppm");
	pixel_t test_pixel;

	img_out = load_ppm("img.ppm");

	int nb_threads = 6000;
	pthread_t *threads = malloc(sizeof(pthread_t) * nb_threads);//(pthread_t*)malloc(sizeof(pthread_t)); //malloc(sizeof(pthread_t) * nb_threads);
	param_t *threads_param = malloc(sizeof(param_t) * nb_threads);//(param_t*)malloc(sizeof(param_t));//malloc(sizeof(param_t) * nb_threads);
	char* text;
	int max_char = max_char_encode(img);
	printf("MaxChar : %d\n", max_char);
	int nb_char = fsize("text.txt");
	int nb_char_encoded = nb_char * BITS_PER_CHAR;
	char* text_encoded;
	float interval;
	limit_threads_t limit;
	char *nb_char_header = calloc(32+1, sizeof(char));
	int count_thread = 0;
	pthread_t *pthread_realloc;
	param_t *param_realloc;


	if (nb_char > max_char){
		printf("Fichier texte trop long pour l'image\n");
		exit(0);
	}
		
	int2bin(nb_char, nb_char_header, 32);

	printf("------ nb_char_header %s\n", nb_char_header);

	write_nb_char_in_img(nb_char_header, &img_out);
	

	file_to_str("text.txt", nb_char, &text);
	encode_str(text,nb_char,&text_encoded);
	printf("%s", text_encoded);
	printf("\n\nnb char * 7 -> %u\nstrlen -> %u\n",nb_char*7,strlen(text_encoded));

	interval = (float)nb_char_encoded / (float)nb_threads;
	printf("%f\n\n\n",interval);
	
	test_pixel = img->raw[FIRST_PIXEL];
	printf("[IMG] La valeur RGB du Pixel: %d\n", test_pixel.r);



	for (int i = 0; i < nb_threads; i++){
		int min = round(interval * i) + 0;
		int max = round(interval * (i + 1)) - 1;
		int char_in_interval = max + 1 - min;

		char *text_cut = calloc(char_in_interval + 1, sizeof(char));

		if (char_in_interval > 0){
				memcpy(text_cut, text_encoded + min, char_in_interval);
				text_cut[char_in_interval] = 0;
				
				threads_param[count_thread].limit = get_limits(min);
				printf("initial_indice for %d : %d\n", count_thread, threads_param[count_thread].limit.initial_indice);
				threads_param[count_thread].text_cut = text_cut;
				threads_param[count_thread].thread_id = count_thread;
				threads_param[count_thread].img_out = &img_out;

				int code = pthread_create(&threads[count_thread], NULL, thread, &threads_param[count_thread]);

				if (code != 0){
		            fprintf(stderr, "pthread_create failed!\n");
		            exit(0);
				}
				
			count_thread++;
		}
	}

	for (int i = 0; i < count_thread; i++)
    {
        pthread_join(threads[i], NULL);
    }


    printf("%d\n", 'U');
    show_img(img_out, 50);

    write_ppm("img_out.ppm", img_out, PPM_BINARY);
	return 0;
}
