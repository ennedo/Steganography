#include "encode_lib.h"

void decode_char(char a, char *b){
	for (int i = BITS_PER_CHAR - 1; i >= 0; --i)
        b[BITS_PER_CHAR-i-1] = (a & (1 << i)) ? '1' : '0';
}

uint8_t encode_char(uint8_t rgb, char c){
	rgb >>= 1;
	rgb <<= 1;
	rgb += c - '0';

	return rgb;
}

void encode_str(char* text, int nb_char, char** dest){
	int count = 0;
	char *s = calloc(BITS_PER_CHAR + 1, sizeof(char));
	char *temp = calloc(BITS_PER_CHAR * nb_char + 1, sizeof(char));
	for (int i = 0; i < nb_char; i++){
		char c = text[i];
		decode_char(c, s);
		for (int j = 0; j < BITS_PER_CHAR; j++){
			temp[count] = s[j];
			count++;
		}
	}
	*dest = temp;
	free(s);
}
