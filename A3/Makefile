CC = clang
CFLAGS = -Wall -Wpedantic -Wextra -Werror -g

all: a3

a3: a3.o
	$(CC) $(CFLAGS) -o a3 a3.o

a3.o: a3.c fat32.h DirInfo.h Fsinfo.h
	$(CC) $(CFLAGS) -c a3.c

clean:
	rm -f *.o a3