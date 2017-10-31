#include <sys/stat.h>
#include "encode_lib.h"
#include "ppm.h"
#include <string.h>
#include <errno.h>
#include <pthread.h>
//hello
#define FIRST_PIXEL 11

typedef struct limit_threads_st {
	int initial_indice;
	int initial_pos_rgb;
	int final_indice;
	int final_pos_rgb;
} limit_threads_t;

typedef struct thread_st {
	int thread_id;
	limit_threads_t limit;
	char *text_cut;
	img_t **img_out;
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

    for (int i = buf_size - 1; i >= 0; i--) {
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
	return floor((img->height * img->width - FIRST_PIXEL) * 3 / BITS_PER_CHAR);
}

limit_threads_t get_limits(int min, int max)
{
	limit_threads_t ret;

	ret.initial_indice = floor((float)min/3) + FIRST_PIXEL;
	ret.final_indice = floor((float)max/3) + FIRST_PIXEL;

	ret.initial_pos_rgb = min % 3;
	ret.final_pos_rgb = max % 3;

	//printf("initial_indice : %d reste min %d / final_indice : %d / reste max %d \n", ret.initial_indice, ret.initial_pos_rgb, ret.final_indice, ret.final_pos_rgb);

	return ret;
}


void write_nb_char_in_img(char *nb_char, img_t **img_out)
{
	printf("%s\n", nb_char);

	uint8_t rgb = 0;
	uint8_t *test_rgb = &(*img_out)->raw[0].r;

	//printf("%d\n", *(test_rgb+3));

	for (int i = 0; i < strlen(nb_char); i++)
	{
		rgb = *(test_rgb+i);
		rgb = encode_char(rgb, nb_char[i]);
		*(test_rgb+i) = rgb;
	}	
}

void show_img(img_t *img, int nb_pixel){
    uint8_t component;
    int max_pixel = img->height * img->width;
    uint8_t *ptr = &img->raw[0].r;
    printf("ptr %d",*ptr);
    if (nb_pixel > max_pixel)
        nb_pixel = max_pixel;
    char* temp = calloc(9, sizeof(char));
    printf("\n\n\n--------- image ---------\n");
    for(int i = 0; i < nb_pixel; i++){
        for(int j = 0; j < 3; j++){
            component = *ptr;
            int2bin(component, temp, 8);
            printf("   %3d: %3d %s\n",i,component,temp);
            ptr =  ptr+1;
        }
        printf("\n");
    }   
    printf("--------- image ---------\n\n\n");
}

void *thread(void *para)
{
    thread_t *p = (thread_t *)para;

    printf("hello from thread : %d\n", p->thread_id);

    int initial_ind = p->limit.initial_indice;
    int initial_pos = p->limit.initial_pos_rgb;
    img_t **img_out = p->img_out;

 	printf("initial_indice for thread->%d : %d\n", p->thread_id, initial_ind);
   

    uint8_t rgb = 0;
    uint8_t *test_rgb = &(*img_out)->raw[initial_ind].r + initial_pos;
    
    printf("RGB for threads i=%d : %d\n", p->thread_id, *test_rgb);

    for (int i = 0; i < strlen(p->text_cut); i++)
    {
    	rgb = *(test_rgb+i);
		rgb = encode_char(rgb, p->text_cut[i]);
		*(test_rgb+i) = rgb;
    }
	//
    // int ret = p->first_nb;
 	//printf("RGB for threads i=%d : %d\n", p->thread_id, rgb);
    // do
    // {
    //     ret += p->jump;

    // }while(ret <= p->last_nb);

    // ret -= p->jump;
    // printf("%d\n", ret);
    // return (void *) ret;
}

int main(int argc, char **argv){
	img_t *img, *img_out;
	img = load_ppm("img.ppm");

	pixel_t test_pixel;

	
	
	//img_out = alloc_img(img->width, img->height);
	img_out = load_ppm("img.ppm");


	// write_nb_char_in_img("00000000000000000000001011110100", &img_out);

	// for (int i = 0; i < 11; i++)
	// {
	// 	test_pixel = img->raw[i];
	// 	printf("[IMG] La valeur RGB du Pixel %d : %d\n", i, test_pixel.r);
	// 	printf("[IMG] La valeur RGB du Pixel %d : %d\n", i, test_pixel.g);
	// 	printf("[IMG] La valeur RGB du Pixel %d : %d\n", i, test_pixel.b);
		
	// 	test_pixel = img_out->raw[i];
	// 	printf("[IMG_OUT] La valeur RGB du Pixel %d : %d\n", i, test_pixel.r);
	// 	printf("[IMG_OUT] La valeur RGB du Pixel %d : %d\n", i, test_pixel.g);
	// 	printf("[IMG_OUT] La valeur RGB du Pixel %d : %d\n", i, test_pixel.b);

	// 	printf("-----------------\n");
	// }

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

	for (int i = 0; i < nb_threads; i++)
	{
		
		int min = round(interval * i) + 0;
		int max = round(interval * (i + 1)) - 1;
		int char_in_interval = max + 1 - min;

		char *text_cut = calloc(char_in_interval + 1, sizeof(char));

		if (char_in_interval > 0)
		{
			memcpy(text_cut, text_encoded + min, char_in_interval);
			text_cut[char_in_interval] = 0;
			
			threads_param[i].limit = get_limits(min,max);;
			threads_param[i].text_cut = text_cut;
			threads_param[i].thread_id = i;
			threads_param[i].img_out = &img_out;

			int code = pthread_create(&threads[i], NULL, thread, &threads_param[i]);

			if (code != 0)
	        {
	            fprintf(stderr, "pthread_create failed!\n");
	            //return EXIT_FAILURE;
	            exit(0);
	        }
			
			// printf("thread[%d] : initial_indice : %d reste min %d / final_indice : %d / reste max %d \n", i, threads_param[i].limit.initial_indice, threads_param[i].limit.initial_pos_rgb, threads_param[i].limit.final_indice, threads_param[i].limit.final_pos_rgb);
			// printf("threads_param : %s", threads_param[i].array);
			// printf("\nInterval: %u, Strlen: %u", char_in_interval, strlen(text_cut));
		}
		//printf("\nMin: %u / Max: %u / Interval: %u\n\n", min, max, char_in_interval);
	}

	for (int i = 0; i < nb_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    show_img(img_out, 100);

    write_ppm("img_out.ppm", img_out, PPM_BINARY);
	return 0;
}
