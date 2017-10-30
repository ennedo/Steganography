/**
 * @file ppm.c
 * @author Florent Gluck
 * @date 17 Oct 2017
 * @brief Routines to read and write PPM files.
 *
 * Both binary (P6 type) and plain ASCII (P3 type) PPM file types are supported.
 * The PPM file format is described here: http://netpbm.sourceforge.net/doc/ppm.html
 * To convert a JPG image into a binary PPM file (P6) with ImageMagick:
 * convert image.jpg output.ppm
 * To convert a JPG image into a plain ASCII PPM file (P3) with ImageMagick:
 * convert -compress none image.jpg output.ppm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppm.h"

/**
 * Allocate the memory for an image of size width*height
 * @param width the width of the image to allocate
 * @param height the height of the image to allocate
 * @return a pointer to the allocated image or NULL if the allocation failed
 */
img_t *alloc_img(int width, int height) {
	img_t *img = malloc(sizeof(img_t));
	if (!img) return NULL;
	img->width = width;
	img->height = height;
	img->raw = malloc(sizeof(pixel_t) * width * height);
	if (!img->raw) {
		free(img);
		return NULL;
	}
	img->pix = malloc(sizeof(pixel_t*) * height);
	if (!img->pix) {
		free(img);
		free(img->raw);
		return NULL;
	}
	for (int i = 0; i < height; i++)
		img->pix[i] = img->raw + width*i;
	return img;
}

/**
 * Free an allocated image.
 * @param img a pointer to the image to free
 */
void free_img(img_t *img) {
	free(img->pix);
	free(img->raw);
	free(img);
}

/**
 * Write a 24-bit RGB PPM file (either ASCII P3 type or binary P6 type).
 * @param filename (absolute or relative path) of the image to write
 * @param img a pointer to the image to write
 * @param PPM_TYPE the type of the image to write (binary or ASCII)
 * @return boolean value indicating whether the write succeeded or not
 */
bool write_ppm(char *filename, img_t *img, enum PPM_TYPE type) {
	FILE *f = fopen(filename, "w");
	if (!f) return false;

	if (type == PPM_BINARY) {
		fprintf(f, "%s\n%d %d\n255\n", "P6", img->width, img->height);
		// Write image content
		for (int i = 0; i < img->width * img->height; i++) {
			pixel_t *p = &img->raw[i];
			fwrite(&p->r, sizeof(p->r), 1, f);
			fwrite(&p->g, sizeof(p->g), 1, f);
			fwrite(&p->b, sizeof(p->b), 1, f);
		}
	} else {
		fprintf(f, "%s\n%d %d\n255\n", "P3", img->width, img->height);
		// Write image content
		int count = 0;
		for (int i = 0; i < img->width * img->height; i++) {
			pixel_t *p = &img->raw[i];
			fprintf(f, "%d %d %d ", p->r, p->g, p->b);
			if (++count % 5 == 0)  // New line every 5 pixels (max 70 characters/line)
				fprintf(f, "\n");
		}
	}

	fclose(f);
	return true;
}

/**
 * Internal routine to parse a PPM header.
 * @param filename (absolute or relative path) of the image to load
 * @param type the type of the PPM file; caller is responsible for allocating space for type (3 characters)
 * @param width the width of the image
 * @param height the height of the image
 * @param maxval the maximum value per component
 * @return a pointer to the opened image file
 */
static FILE* load_header(char *filename, char *type, unsigned int *width, unsigned int *height,
                  unsigned int *maxval) {
	FILE *f = fopen(filename, "r");
	if (f == NULL) return NULL;
	
	// Reads the PPM header, ensuring it's valid
	int matches;
	// File type (format) must be P3 or P6
	matches = fscanf(f, "%2s", type);  // read 2 characters
	type[2] = 0;
	if (matches != 1) goto error;
	// Image width and height
	matches = fscanf(f, "%u %u", width, height);
	if (matches != 2) goto error;
	// Maximum value per component
	matches = fscanf(f, "%u ", maxval);
	if (matches != 1) goto error;
	if (*maxval > 255) {
		fprintf(stderr, "PPM reader: doesn't support more than 1 byte per component!\n");
		goto error;
	}
	return f;

error:
	fclose(f);
	return NULL;
}

/**
 * Load a 24-bit RGB PPM file (either ASCII P3 type or binary P6 type).
 * The routine takes care of allocating the memory for the image.
 * @param filename (absolute or relative path) of the image to load
 * @return a pointer to the loaded image or NULL if an error occured
 */
img_t *load_ppm(char *filename) {
	unsigned int width, height, maxval;
	char type[3];
	FILE *f = load_header(filename, type, &width, &height, &maxval);
	if (!f) return NULL;

	// Allocate memory for image structure and image data
	img_t *img = alloc_img(width, height);
	if (!img) return NULL;

	// File type (format) must be P3 or P6
	if (strcmp("P3", type) == 0) {
		// Image data in RGB order, ASCII encoded 
		for (unsigned int i = 0; i < width * height; i++) {
			unsigned int r, g, b;
			int matches = fscanf(f, "%u %u %u", &r, &g, &b);
			if (matches != 3) goto error;
			if (r > maxval || g > maxval || b > maxval) goto error;
			pixel_t p = { r, g, b };
			img->raw[i] = p;
		}
	}
	else if (strcmp("P6", type) == 0) {
		// Image data in RGB order, binary encoded 
		for (unsigned int i = 0; i < width * height; i++) {
			fread(&img->raw[i].r, 1, 1, f);
			fread(&img->raw[i].g, 1, 1, f);
			fread(&img->raw[i].b, 1, 1, f);
		}
	} else {
		fprintf(stderr, "PPM reader: unsupported format!\n");
		goto error;
	}

	fclose(f);
	return img;

error:
	fclose(f);
	return NULL;
}
