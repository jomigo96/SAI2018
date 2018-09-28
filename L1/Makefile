CC=gcc
CFLAGS= -Wall -pedantic 
LDFLAGS= -lm
EXECUTABLE=prog1
SOURCES=prog1.c

OBJECTS=$(notdir $(SOURCES:.c=.o))

all: $(SOURCES) $(EXECUTABLE)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean 
