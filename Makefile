# Makefile for CS 118 Project 2

# Global variables
CC = g++
CCFLAGS = -c -Wall

# Define executable name
BIN = dv_routing

# Define source files
SOURCES = main.cpp my-router.cpp

# Define objects
OBJS = $(SOURCES:.cpp=.o)

# Main entry point
all: $(BIN)

# For linking object file(s) to produce the executable
$(BIN): $(OBJS)
	$(CC) $^ -o $@

# For compiling source file(s)
%.o: %.cpp
	$(CC) $(CCFLAGS) $< -o $@

# For cleaning up the project
clean:
	rm $(OBJS) $(BIN)
