CFLAGS=-Wall -g
CC=gcc

all: clean netspeed

netspeed: 
	$(CC) $(CFLAGS) -o netspeed netspeed.c
clean:
	rm -rf netspeed
