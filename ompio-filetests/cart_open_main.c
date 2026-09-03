#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int cart_comm_open_2D( MPI_Comm comm, int root );
int cart_comm_open_3D( MPI_Comm comm, int root );

int main ( int argc, char ** argv )
{
    int rank, size, newsize, root=0;
    MPI_Comm comm;
    int i=0;
    int color;
    int lowerbound=0;

    if ( argc > 1 ) {
        lowerbound = atoi(argv[1]);
    }

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    for ( i=size; i>lowerbound; i-- ) {
        color = 1;
        if ( rank >= i ) {
            color=MPI_UNDEFINED;
        }
        MPI_Comm_split ( MPI_COMM_WORLD, color, rank, &comm);

        if ( comm != MPI_COMM_NULL ) {
            MPI_Comm_size ( comm, &newsize );

            if ( rank == 0 ) {
                printf("Communicator size: %d\n", newsize);
            }

            cart_comm_open_2D ( comm, root );
            cart_comm_open_3D ( comm, root );
            MPI_Comm_free ( & comm);
        }
    }
    
    MPI_Finalize ();
    return 0;
}
