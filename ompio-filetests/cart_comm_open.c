#include <stdio.h>
#include <unistd.h>
#include "mpi.h"

int cart_comm_open_2D ( MPI_Comm comm, int root )
{
    int size, rank, ret;
    int localret, globalret;
    MPI_File file1=MPI_FILE_NULL;
    int dims2d[2]={0,0}, periods2d[2] ={0,0}, reorder=0 ;
    int ndims=2;
    MPI_Comm cart_comm;
    
    MPI_Comm_size ( comm, &size );
    MPI_Comm_rank ( comm, &rank );

    /**********************************************************************/ 
    if ( rank == root ) {
	printf("    using 2-D cartesian communicators......");
    }
    /* The cartesian communicator tests mostly make sense if you can look 
       internally into the ompio cart_based_grouping function. At this level,
       it is just expected to not segfault :-) 
    */

    MPI_Dims_create ( size, ndims, dims2d );
    MPI_Cart_create ( comm, ndims, dims2d, periods2d, reorder, &cart_comm );
 
    ret = MPI_File_open ( cart_comm, "testfile1.out",MPI_MODE_CREATE| MPI_MODE_DELETE_ON_CLOSE|MPI_MODE_WRONLY, 
			  MPI_INFO_NULL,   &file1 );
    MPI_File_close ( &file1 );

    if ( ret == MPI_SUCCESS ) 
	localret = 0;
    else
	localret = 1;
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
    }
    MPI_Comm_free ( &cart_comm );

    return MPI_SUCCESS;
}


int cart_comm_open_3D ( MPI_Comm comm, int root )
{
    int size, rank, ret;
    int localret, globalret;
    MPI_File file1=MPI_FILE_NULL;
    MPI_Comm cart_comm;
    int dims3d[3]={0,0,0}, periods3d[3] ={0,0,0}, reorder=0;
    int ndims=3;

    MPI_Comm_size ( comm, &size );
    MPI_Comm_rank ( comm, &rank );


    /**********************************************************************/ 
    if ( rank == root ) {
	printf("    using 3-D cartesian communicators......");
    }
    /* The cartesian communicator tests mostly make sense if you can look 
       internally into the ompio cart_based_grouping function. At this level,
       it is just expected to not segfault :-) 
    */
    MPI_Dims_create ( size, ndims, dims3d );
    MPI_Cart_create ( comm, ndims, dims3d, periods3d, reorder, &cart_comm );
 
    ret = MPI_File_open ( cart_comm, "testfile1.out",MPI_MODE_CREATE| MPI_MODE_DELETE_ON_CLOSE|MPI_MODE_WRONLY, 
			  MPI_INFO_NULL,   &file1 );
    MPI_File_close ( &file1 );

    if ( ret == MPI_SUCCESS ) 
	localret = 0;
    else
	localret = 1;
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
    }
    MPI_Comm_free ( &cart_comm );

  return MPI_SUCCESS;
}




