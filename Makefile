CC = gcc
CFLAGS = -ansi -Wall -pedantic -lpthread
TARGETS = Assignment1

all: $(TARGETS)

clean:
	rm -fr $(TARGETS) *~ *.o
