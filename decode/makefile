GCC=gcc -g -Wall -Wextra -std=gnu11
LIBS=-lm -lpthread
decode: decode.o decode_lib.o ppm.o alloc.o
	$(GCC) $^ -o $@ $(LIBS)
decode.o: decode.c
	$(GCC) $< -c
decode_lib.o: decode_lib.c decode_lib.h
	$(GCC) $< -c
ppm.o: ../libs/ppm.c ../libs/ppm.h
	$(GCC) $< -c
alloc.o: ../libs/alloc.c ../libs/alloc.h
	$(GCC) $< -c
run: decode
	./decode
clean:
	rm -f *.o decode; clear
