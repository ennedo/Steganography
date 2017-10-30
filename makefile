GCC=gcc -g -Wall -Wextra -std=gnu11 -lpthread

encode: encode.o encode_lib.o ppm.o
	$(GCC) $^ -o $@
encode.o: encode.c
	$(GCC) $< -c
encode_lib.o: encode_lib.c encode_lib.h
	$(GCC) $< -c
ppm.o: ppm.c ppm.h
	$(GCC) $< -c
run: encode
	./encode
clean:
	rm -f *.o encode; clear

rebuild: 
	clean encode
