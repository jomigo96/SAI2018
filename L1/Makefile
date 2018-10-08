CC=gcc
CFLAGS= -Wall -pedantic 
LDFLAGS= -lm
EXECUTABLE=prog
SOURCES=prog1.c
INPUT=routes/vhhh-klax.txt

OBJECTS=$(notdir $(SOURCES:.c=.o))

all: $(SOURCES) $(EXECUTABLE)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)

memcheck: $(EXECUTABLE)
	valgrind --leak-check=full --show-leak-kinds=all ./$< $(INPUT)

run: $(EXECUTABLE)
	./$< $(INPUT)

.PHONY: all clean run
