CC = gcc
CFLAGS = -Wall -pthread

all: cv

cv: main.c
	$(CC) $(CFLAGS) -o cv main.c

clean:
	rm -f cv