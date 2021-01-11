CC = gcc
CFLAGS = -Wall -I.
DEPS = util.h
OBJ = main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: main

main_cmp: main.o
	$(CC) -o $@ $^ $(CFLAGS)

main: main_cmp
	./main_cmp output.txt

clean:
	$(RM) count *.o main_cmp output*