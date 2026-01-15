CC = gcc
CFLAGS = -Wall -g -pthread

all: butterdb

butterdb: dbserver.c btree.c pager.c wal.c
	$(CC) $(CFLAGS) dbserver.c btree.c pager.c wal.c -o butterdb

test_phase2: test_phase2.c btree.c pager.c wal.c
	$(CC) $(CFLAGS) test_phase2.c btree.c pager.c wal.c -o test_phase2

clean:
	rm -f butterdb test_phase2 btree.dat butterdb.wal
