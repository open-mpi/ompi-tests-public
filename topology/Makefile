MPICC = mpicc
CFLAGS = -Wall -g -O2

PROGS = neighbor_alltoall_conformance dist_graph_constructors

all: $(PROGS)

neighbor_alltoall_conformance: neighbor_alltoall_conformance.c
	$(MPICC) neighbor_alltoall_conformance.c $(CFLAGS) -o neighbor_alltoall_conformance

dist_graph_constructors: dist_graph_constructors.c
	$(MPICC) dist_graph_constructors.c $(CFLAGS) -o dist_graph_constructors

clean:
	$(RM) $(PROGS)
