CC = clang
CFLAGS = -Wall -Wpedantic -Wextra -Werror

all: elfRead.o process.o threads.o elfRead process threads

elfRead.o: elfRead.c
	$(CC) $(CFLAGS) -c elfRead.c -o elfRead.o

process.o: process.c
	$(CC) $(CFLAGS) -c process.c -o process.o

threads.o: threads.c
	$(CC) $(CFLAGS) -c threads.c -o threads.o

elfRead: elfRead.o
	$(CC) $(CFLAGS) elfRead.o -o elfRead
	rm -f elfRead.o

process: process.o
	$(CC) $(CFLAGS) process.o -o process
	rm -f process.o

threads: threads.o
	$(CC) $(CFLAGS) threads.o -o threads
	rm -f threads.o

clean:
	rm -f elfRead process threads
