#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "mpi.h"

int main(int argc, char* argv[])
{
    int mcw_rank, mcw_size, len;
    char name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &mcw_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mcw_size);
    MPI_Get_processor_name(name, &len);

    MPI_Barrier( MPI_COMM_WORLD );

    printf("%3d/%3d) [%s] %d Hello, world!\n",
           mcw_rank, mcw_size, name, (int)getpid());
    fflush(NULL);

    MPI_Finalize();

    return 0;
}
