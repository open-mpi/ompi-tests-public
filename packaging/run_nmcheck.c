/*
 * $HEADER$
 *
 * Run nmcheck_prefix.pl on itself
 * only needs to be run with one rank
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
    char cmd[256];
    int myrank;
    int rv;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    if (myrank == 0) {
        sprintf(cmd, "./nmcheck_prefix.pl \"%s\"", argv[0]);
        rv = system(cmd);
        if (rv) { MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER); }
    }
    MPI_Finalize();
    return 0;
}
