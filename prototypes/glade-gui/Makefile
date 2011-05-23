
DEPS=gtk+-2.0 libglade-2.0
LIBS=`pkg-config --libs $(DEPS)` 
CFLAGS=`pkg-config --cflags $(DEPS)` 

all: run

run: glade-proto.c
	gcc -Wall -g -o run glade-proto.c -export-dynamic $(LIBS) $(CFLAGS)
