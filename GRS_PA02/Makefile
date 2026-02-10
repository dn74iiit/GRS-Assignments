# MT25074
# Roll number - MT25074
# Name - Nindra Dhanush
# I'm using a single Makefile to build all Part A binaries so the Part C script can run "make all".

# Makefile for Part A: A1 (Two-Copy), A2 (One-Copy), A3 (Zero-Copy)

CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2

# All binaries
TARGETS = MT25074_Part_A1_Server MT25074_Part_A1_Client \
          MT25074_Part_A2_Server MT25074_Part_A2_Client \
          MT25074_Part_A3_Server MT25074_Part_A3_Client

# Default: build all
all: $(TARGETS)

# Pattern rules: .c -> executable (no .o files needed)
%_Server: %_Server.c
	$(CC) $(CFLAGS) -o $@ $<

%_Client: %_Client.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean all binaries
clean:
	rm -f $(TARGETS)

