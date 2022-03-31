CC=g++

TARGETS=client
CFLAGS=-std=gnu++17 -pedantic -Wall -Werror -ggdb3 -pthread
EXTRAFLAGS=-lpqxx -lpq

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

client: client.o test_funcs.o
	g++ -o $@ $^ -lpthread $(EXTRAFLAGS)

client.o: client.cpp  
	g++ $(CFLAGS) -c  $< 

test_funcs.o: test_funcs.cpp test_funcs.h 
	g++ $(CFLAGS) -c  $< 
