CC=g++

TARGETS=server client
CFLAGS=-std=gnu++17 -pedantic -Wall -Werror -ggdb3 -pthread
EXTRAFLAGS=-lpqxx -lpq

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

server: server.cpp request.cpp 
	g++ $^ -o $@ -lpthread $(EXTRAFLAGS)

client: client.cpp
	g++ -o $@ $<
