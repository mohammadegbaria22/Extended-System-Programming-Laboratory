all: myshell mypipe Looper

myshell: myshell.o LineParser.o
	gcc -g -m32 -Wall -o myshell myshell.o LineParser.o

myshell.o: myshell.c
	gcc -g -m32 -Wall -c myshell.c

Looper: Looper
	gcc Looper.c -o Looper

LineParser.o: LineParser.c
	gcc -g -m32 -Wall -c LineParser.c

mypipe: mypipe.o
	gcc -g -m32 -Wall -o mypipe mypipe.o

mypipe.o: mypipe.c
	gcc -g -m32 -Wall -c mypipe.c

clean:
	rm -f *.o myshell mypipe Looper