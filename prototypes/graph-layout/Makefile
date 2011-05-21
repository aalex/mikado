all: gl

gl: graph_layout.o

graph_layout.o: graph_layout.c
	gcc -Wall -Werror `pkg-config --libs --cflags clutter-1.0` -c -o graph_layout.o graph_layout.c

DEPS=clutter-1.0
CFLAGS=`pkg-config --cflags $(DEPS)`
LIBS=`pkg-config --libs $(DEPS)`

all: run


run: graph_layout.o main.o
	gcc -Wall -g $(LIBS) graph_layout.o main.o -o run

graph_layout.o: graph_layout.c
	gcc -Wall -g $(CFLAGS) graph_layout.c -c -o graph_layout.o

main.o: main.c
	gcc -Wall -g $(CFLAGS) main.c -c -o main.o

.PHONY: clean

clean:
	rm -f run *.o


