CC = gcc
CFLAGS = -pedantic -Wall -std=gnu99 
LDFLAGS = -lm -lGL -lSDL2 -lpthread
DEPTS = ils.h
OBJ = ils.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

ils: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

stub: stub.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o ils

run: ils
	./ils 8080

.PHONY: ils clean run stub
