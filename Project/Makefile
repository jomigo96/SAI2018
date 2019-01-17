CC = gcc
CFlAGS = -pedantic -Wall -ansi 
LDFLAGS = -lm -lGL -lSDL2
DEPTS = ils.h
OBJ = ils.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ils: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o ils

run: ils
	./ils

.PHONY: ils clean
