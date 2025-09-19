

CC=mpicc
CFLAGS=-Wall -g -O0

PROGS=test_mpi_win_shared_query

all: $(PROGS)

test_mpi_win_shared_query: test_mpi_win_shared_query.c
	$(CC) test_mpi_win_shared_query.c $(CFLAGS) -o test_mpi_win_shared_query


clean:
	rm -f *.o $(PROGS)
