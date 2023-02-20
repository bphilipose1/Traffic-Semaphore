CC = g++
CFLAGS = -c -Wall -pthread
LDFLAGS = -pthread

all: p2

p2: p4.o
	$(CC) $(LDFLAGS) p4.o -o p2

p4.o: p4.cpp
	$(CC) $(CFLAGS) p4.cpp

car.log:
	touch car.log

flagperson.log:
	touch flagperson.log

clean:
	rm -rf *o p2 car.log flagperson.log
