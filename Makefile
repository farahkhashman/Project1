# Makefile for client.c and server.c
CC = gcc
CFLAGS = -g -Wall -std=c99

%.o:	%.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@echo Cleaning...
	@echo 
	@rm client
	@rm server