/*
** Test for MPI File I/O Operations
*/

#include <stdio.h>
#include <unistd.h>
#include "mpi.h"


/* Some prototype declarations  */
int open_test ( MPI_Comm, int );
int utils_test ( MPI_Comm, int );
int write_test ( MPI_Comm, int );
int read_test ( MPI_Comm, int );
int fileview_test ( MPI_Comm, int );
int write_all_test ( MPI_Comm, int );
int read_all_test ( MPI_Comm, int );
int write_shared_test ( MPI_Comm, int );
int read_shared_test ( MPI_Comm, int );
int iwrite_test ( MPI_Comm, int );
int iread_test ( MPI_Comm, int );
int info_test ( MPI_Comm, int );
int atomicity_test ( MPI_Comm, int );



int main ( int argc, char * argv[] )
{

    int mynode, root, numnode;
  
    /* This file only does the global initialisation and
    ** calls the routines
    */
    MPI_Init ( &argc, &argv );
    MPI_Comm_size ( MPI_COMM_WORLD, &numnode );
    MPI_Comm_rank ( MPI_COMM_WORLD, &mynode );

    if ( argc > 1 ) sleep ( 20 );


    /* 
    ** We always have to define a root, even for 
    ** operations which doesnt need a root, because
    ** of the output
    */
    root = 0;

    /*
    ** This test works only correctly with six processes!
    ** Not very nice, but some tests are difficult
    ** to evaluate for correctness  not knowing the precise
    ** number of processes
    */

    if (numnode != 6 && mynode == root)
    {
	printf("Sorry, this test only works correctly with 6 processes\n");
	MPI_Abort ( MPI_COMM_WORLD, 1);
    }
    
    int total = 0;

    /* The open test, contains also close */
    total += open_test ( MPI_COMM_WORLD, root);

    /* The write test, contains also write_at */
    total += write_test ( MPI_COMM_WORLD, root);

    /* The read test, contains also read_at */
    total += read_test ( MPI_COMM_WORLD, root);

    /* The fileview test uses both read and write */
    total += fileview_test ( MPI_COMM_WORLD, root);

    /* The write_all test also tests write_at_all */
    total += write_all_test ( MPI_COMM_WORLD, root);

    /* The read_all test also tests read_at_all */
    total += read_all_test ( MPI_COMM_WORLD, root);

    /* The iwrite test, contains also iwrite_at */
    total += iwrite_test ( MPI_COMM_WORLD, root);

    /* The iread test, contains also iread_at */
    total += iread_test ( MPI_COMM_WORLD, root);

    /* The write_shared test also tests write_ordered */
    total += write_shared_test ( MPI_COMM_WORLD, root);

    /* The read_shared test also tests read_ordered */
    total += read_shared_test ( MPI_COMM_WORLD, root);

    /* Test various utility functions in MPI I/O */
    total += utils_test ( MPI_COMM_WORLD, root);

    /* Test various Info objects with MPI I/O */
    total += info_test ( MPI_COMM_WORLD, root);

    /* Test atomicity function */
    total += atomicity_test ( MPI_COMM_WORLD, root);

    MPI_Finalize ();
    return ( total > 0 ) ? 1 : 0;
}












