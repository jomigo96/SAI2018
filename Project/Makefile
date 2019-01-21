CC = gcc
CFLAGS = -pedantic -Wall -std=c99 -I/usr/include/SDL2
LDFLAGS = -lm -lGL -lSDL2 -lpthread -lSDL2_ttf
DEPTS = ils.h reception_thread.h
OBJ = ils.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: ils stub

ils: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o ils stub

run: ils
	./ils runways.txt 8080

stub: stub.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: ils clean run stub all
