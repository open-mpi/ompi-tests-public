/*
** This file test MPI_File_read 
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mpi.h"
#include "common.h"

#ifdef GLOBAL
int read_test ( MPI_Comm comm, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
#endif

    int rank, size, i, ret, localret, globalret;
    int *bigarr=NULL;
    MPI_Status status;
    MPI_File file1;
    int readarr[6];

    
#ifndef GLOBAL
    MPI_Init ( &argc, &argv );
    if ( argc > 1 ) sleep ( 20 );
#endif

    MPI_Comm_size ( comm, &size );
    MPI_Comm_rank ( comm, &rank );
    
#ifndef GLOBAL
    if ( size != 6 ) {
	printf("Sorry, this test only works correctly with 6 processes\n");
	MPI_Abort ( MPI_COMM_WORLD, 1);
    }
#endif


    if ( rank == root ) {
	int fh, writearr[36];
	int old_mask, perm;

	for ( i=0; i<36; i++ ) {
	    writearr[i] = i;
	}

	printf("Checking for MPI_File_read:\n");
	printf("    with default file view.................");

	/* generate a file first. */
	old_mask = umask(022);
	umask (old_mask);
	perm = old_mask^0666;

	fh = open ("readfile1.out", O_CREAT|O_WRONLY, perm );
	if ( -1 == fh ) {
	    printf("Could not create input file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    SL_write ( fh, writearr, 36 * sizeof(int));
	    close (fh);
	}
    }

    MPI_Barrier ( comm );
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );
    ret = MPI_File_read ( file1, readarr, 6, MPI_INT, &status);
    MPI_File_close ( &file1);

    localret = 0;
    for (i = 0 ; i<6; i++ ) {
	if (readarr[i] != i ) {
	    localret = 1;
#ifdef VERBOSE
	    printf("%d: element %d is %d\n", rank, i, readarr[i] );
#endif
	}
    }
	
    if ( ret != MPI_SUCCESS ) {
	localret = 1;
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	int old_mask, perm, fh;
	int count;

	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
	printf("    verifying status fields................");
	MPI_Get_elements ( &status, MPI_INT, &count );
	if ( count == 6 ) 
	    printf("working\n");
	else
	    printf("false\n");

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

	printf("    reading a large file...................");
	bigarr = (int *) malloc ( 12 * 1024 * 1024 * sizeof(int));
	if ( NULL == bigarr)  {
	    printf("Could not allocate memory\n");
	    MPI_Abort(comm, 1);
	}
	for ( i=0; i<(12*1024*1024); i++ ) {
	    bigarr[i] = i;
	}
	/* generate a file first. */
	old_mask = umask(022);
	umask (old_mask);
	perm = old_mask^0666;

	fh = open ("bigfile.out", O_CREAT|O_WRONLY, perm );
	if ( -1 == fh ) {
	    printf("Could not create input file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    SL_write ( fh, bigarr, 12*1024*1024*sizeof(int));
	    close (fh);
	}
	memset ( bigarr, 0, 12*1024*1024*sizeof(int) );

	MPI_File_open ( MPI_COMM_SELF, "bigfile.out", MPI_MODE_RDONLY, 
			MPI_INFO_NULL, &file1 );
	MPI_File_read ( file1, bigarr, 12*1024*1024, MPI_INT, &status );
	MPI_File_close ( &file1);

	localret = 0;
	for ( i=0; i<(12*1024*1024); i++ ) {
	    if ( bigarr[i] != i ) {
		localret = 1;
#ifdef VERBOSE
                printf("[%d]: element %d is %d should be %d\n", rank, i, bigarr[i], i);
#endif
	    }
	}

	if ( localret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	unlink ( "bigfile.out");

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
	printf("Checking for MPI_File_read_at..............");
    }

    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );

    ret = MPI_File_read_at ( file1, rank*sizeof(int)*6, readarr, 6, MPI_INT, &status);
    MPI_File_close (&file1);

    for (i = 0; i<6; i++ ) {
	if (readarr[i] != rank*6+i ) 
	    localret = 1;
    }
	
    if ( ret != MPI_SUCCESS ) {
	localret = 1;
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");

       /* clean up the files generated in this test case. */
	unlink ("readfile1.out");
    }

#ifndef GLOBAL
    MPI_Finalize ();
#endif

    return 0;
}
