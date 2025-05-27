
all : menu_map

menu_map : menu_map.o
	gcc -g -Wall -m32 -o menu_map menu_map.o

menu_map.o : menu_map.c
	gcc -c -g -Wall -m32 -o menu_map.o menu_map.c

clean:
	rm -f *.o menu_map