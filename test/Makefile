CC=g++

TARGETS=test_scalability test_init
CFLAGS=-std=gnu++17 -pedantic -Wall -Werror -ggdb3 -pthread
EXTRAFLAGS=-lpqxx -lpq

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

test_init: test_init.o test_funcs.o client.o
	g++ -o $@ $^ -lpthread $(EXTRAFLAGS)

test_scalability: test_scalability.o test_funcs.o client.o
	g++ -o $@ $^ -lpthread $(EXTRAFLAGS)

test_scalability.o: test_scalability.cpp
	g++ $(CFLAGS) -c  $< 

test_funcs.o: test_funcs.cpp test_funcs.h 
	g++ $(CFLAGS) -c  $< 

client.o: client.cpp client.h
	g++ $(CFLAGS) -c  $< 

test_init.o: test_init.cpp
	g++ $(CFLAGS) -c  $< 
