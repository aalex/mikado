DEPS=gegl
CFLAGS=`pkg-config --cflags $(DEPS)`
LIBS=`pkg-config --libs $(DEPS)`

all: run

run: main.o
	gcc -Wall -g $(LIBS) main.o -o run

main.o: main.c
	gcc -Wall -g $(CFLAGS) main.c -c -o main.o

.PHONY: clean

clean:
	rm -f run *.o


