CC = gcc
CFLAGS = -Wall -O2

# Change target name to match your file
all: dbserver

dbserver: dbserver.c btree.c
	$(CC) $(CFLAGS) dbserver.c btree.c -o butterdb

clean:
	rm -f butterdb btree.dat
