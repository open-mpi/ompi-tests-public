CC=mpicc
CFLAGS=-Wall -g -O0

PROGS=hello_c simple_spawn simple_spawn_multiple

all: $(PROGS)

hello_c: hello_c.c
	$(CC) hello_c.c $(CFLAGS) -o hello_c

simple_spawn: simple_spawn.c
	$(CC) simple_spawn.c $(CFLAGS) -o simple_spawn

simple_spawn_multiple: simple_spawn_multiple.c
	$(CC) simple_spawn_multiple.c $(CFLAGS) -o simple_spawn_multiple

clean:
	$(RM) $(PROGS) *.o
