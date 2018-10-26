CC = gcc -lm
CFLAGS = -std=c99 -Wall -pedantic -lpthread
TARGETS = Assignment1

all: $(TARGETS)

clean:
	rm -fr $(TARGETS) *~ *.o

