CC = gcc
CFLAGS = -pedantic -Wall -std=c99 -I/usr/include/SDL2 -D_POSIX_C_SOURCE 
LDFLAGS = -lm -lGL -lSDL2 -lpthread -lSDL2_ttf
OBJ = ils.o main.o

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

all: ils stub

ils: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o ils stub main

run: ils
	./ils runways.txt 8080

stub: stub.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: ils clean run stub all
