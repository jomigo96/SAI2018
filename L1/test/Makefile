CC=g++
CPPFLAGS=-c -g -std=c++0x -Wall -Wextra -pedantic
LDFLAGS=-lm
EXECUTABLE=test
SOURCES=$(wildcard *.cpp)
OBJECTS=$(notdir $(SOURCES:.cpp=.o))
OBJ=*.o
LPATH=$(shell pwd)/../fms

$(EXECUTABLE): $(OBJECTS)
	$(MAKE) -C ../fms
	$(CC) -o $@ $< -L$(LPATH) -lfms -Wl,-rpath=$(LPATH) $(LDFLAGS) 

clean:
	rm -rf $(OBJ) $(EXECUTABLE) 

run: $(EXECUTABLE)
	./$(EXECUTABLE)

.PHONY: all doc clean
