# Name: Deepey Panchal
# U-Number: U80305771
# NetID: deepeypradippanchal
# Description: Makefile for cv

# Compiler
CC = gcc
# Compiler flags
CFLAGS = -Wall -Wextra -pthread

# Targets
all: cv

# cv target
cv: main.c
	$(CC) $(CFLAGS) -o cv main.c

# Clean target
clean:
	rm -f cv