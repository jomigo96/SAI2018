CC=gcc
CFLAGS= -Wall -pedantic -g
LDFLAGS= -lm
EXECUTABLE=main
SOURCES=fms.c
HEADERS=fms.h
LPATH=$(shell pwd)
INPUT=../routes/kord-lppt.txt

OBJECTS=$(notdir $(SOURCES:.c=.o))

all: $(SOURCES) $(HEADERS) $(EXECUTABLE)

%.so: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -fPIC -c $<
	$(CC) -shared -Wl,-soname,libfms.so -o libfms.so $(OBJECTS)
	rm -rf $(OBJECTS)

$(EXECUTABLE): libfms.so main.c
	$(CC) $(CFLAGS) -c -o main.o main.c
	$(CC) -o $@ main.o -L$(LPATH) -lfms -Wl,-rpath=$(LPATH) $(LDFLAGS)

clean:
	rm -rf main.o $(OBJECTS) $(EXECUTABLE) libfms.so

memcheck: $(EXECUTABLE)
	valgrind --leak-check=full --show-leak-kinds=all ./$< $(INPUT)

run: $(EXECUTABLE)
	./$< $(INPUT)

.PHONY: all clean run memcheck
