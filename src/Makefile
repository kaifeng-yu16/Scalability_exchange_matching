CC=g++

TARGETS=server
CFLAGS=-std=gnu++17 -pedantic -Wall -Werror -ggdb3 -pthread
EXTRAFLAGS=-lpqxx -lpq

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

server: server.cpp request.cpp threadpool.cpp 
	g++ $^ -o $@ -lpthread $(EXTRAFLAGS)

