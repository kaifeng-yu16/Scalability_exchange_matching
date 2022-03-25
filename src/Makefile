CC=g++

TARGETS=request
CFLAGS=-std=gnu++17 -pedantic -Wall -Werror -ggdb3 -pthread
EXTRAFLAGS=-lpqxx -lpq

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

request: request.o 
	g++ -o $@ $^ -lpthread $(EXTRAFLAGS)

request.o: request.cpp  
	g++ $(CFLAGS) -c  $< 

