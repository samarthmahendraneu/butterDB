all: dbserver

dbserver: dbserver.c kvstore.c
	$(CC) -Wall -O2 dbserver.c kvstore.c -o dbserver

clean:
	rm -f dbserver
