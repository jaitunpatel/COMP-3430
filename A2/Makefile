CC = clang
CFLAGS = -Wall -Wpedantic -Wextra -Werror -g

all: a2

a2: a2.o queue.o
	$(CC) $(CFLAGS) -o a2 a2.o queue.o

a2.o: a2.c queue.h
	$(CC) $(CFLAGS) -c a2.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c

clean:
	rm -f *.o a2
