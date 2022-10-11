CC = g++
CPPFLAGS = -g -Wall -I

all: server subscriber

server: server.cpp
	$(CC) $(CPPFLAGS) -c server.cpp -o server

subscriber: subscriber.cpp
	$(CC) $(CPPFLAGS) -c subscriber.cpp -o subscriber

clean:
	rm -f core server subscriber
