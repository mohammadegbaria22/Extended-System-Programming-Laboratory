CC = gcc
CFLAGS = -m32 -Wall -D_FILE_OFFSET_BITS=64

all: hexeditplus task4.o task4

hexeditplus: task1.c
	$(CC) $(CFLAGS) -o hexeditplus task1.c

task4.o: task4.c
	$(CC) $(CFLAGS) -fno-pie -fno-stack-protector -c task4.c -o task4.o

task4: task4.c
	$(CC) $(CFLAGS) -fno-pie -fno-stack-protector -o task4 task4.c

clean:
	rm -f hexeditplus task4 task4.o
