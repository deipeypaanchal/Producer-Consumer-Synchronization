# Makefile

# Compiler and flags
CC = gcc
CFLAGS = -Wall -pthread

# Default target
all: cv

# Link object files to create the executable
cv: main.o producer.o consumer.o
	$(CC) $(CFLAGS) -o cv main.o producer.o consumer.o

# Compile main.c
main.o: main.c shared.h
	$(CC) $(CFLAGS) -c main.c

# Compile producer.c
producer.o: producer.c shared.h
	$(CC) $(CFLAGS) -c producer.c

# Compile consumer.c
consumer.o: consumer.c shared.h
	$(CC) $(CFLAGS) -c consumer.c

# Clean up generated files
clean:
	rm -f cv *.o