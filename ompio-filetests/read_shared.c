/*
** This file test MPI_File_read_shared and MPI_File_read_ordered
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

static int intcompare(const void *, const void *);

#ifdef GLOBAL
int read_shared_test ( MPI_Comm comm, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
#endif

    int rank, size, i, j, k, ret1, ret2, ret3, localret, globalret;
    MPI_Datatype tmp2, fview2, dats[2];
    int blength[2];
    MPI_Aint displs[2];
    MPI_Status status, stats[3];
    MPI_Request reqs[3];
    MPI_File file1;
    int inarr[12], count;
    int veriarr[72];

#ifndef GLOBAL
    MPI_Init ( &argc, &argv );
    if ( argc > 1 )  sleep ( 20 );
#endif
    MPI_Comm_size ( comm, &size );
    MPI_Comm_rank ( comm, &rank );
    
#ifndef GLOBAL
    if ( size != 6 ) {
	printf("Sorry, this test only works correctly with 6 processes\n");
        MPI_Abort ( MPI_COMM_WORLD, 1);
    }
#endif

    /* create the input file */
    if ( rank == root ) {
	int fh, old_mask, perm;

	for ( i=0; i<72; i++ ) {
	    veriarr[i] = i;
	}

	old_mask = umask(022);
	umask (old_mask);
	perm = old_mask^0666;

	fh = open ("readfile1.out", O_CREAT|O_WRONLY, perm );
	if ( -1 == fh ) {
	    printf("Could not create input file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    ssize_t r;
	    r = write ( fh, veriarr, 72 * sizeof(int));
	    if ( r != 72 * sizeof(int)) {
		printf("Could not create input file. Aborting\n");
		MPI_Abort ( comm, 1);
	    }
	    close (fh);
	}
    }

    MPI_Barrier ( comm );

    if ( rank == root ) {
	printf("Checking for MPI_File_read_shared..........");
    }

    memset (inarr, 0, 12*sizeof(int)); 
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );
    
    ret1 = MPI_File_read_shared ( file1, inarr, 4, MPI_INT, &status );
    ret2 = MPI_File_read_shared ( file1, &inarr[4], 4, MPI_INT, &status );
    ret3 = MPI_File_read_shared ( file1, &inarr[8], 4, MPI_INT, &status );
    MPI_File_close ( &file1);
    
    localret = 0;
    if ( ret1 != MPI_SUCCESS || ret2 != MPI_SUCCESS || ret3 != MPI_SUCCESS) {
	localret = 1;
    }
    /* verifying input data */
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    MPI_Gather (inarr, 12, MPI_INT, veriarr, 12, MPI_INT, root, comm);
    if ( rank == root ) {
	qsort ((int*)veriarr, 72, sizeof(int), intcompare);
	for ( i=0; i< 72; i++ ) {
	    if (veriarr[i] != i ) {
		globalret = 1;
#ifdef VERBOSE
		printf("Element %d is %d\n", i, veriarr[i]);
#endif
	    }
	}

	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	}
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	printf("    verifying status field.................");
	MPI_Get_elements ( &status, MPI_INT, &count );
	if ( count == 4 ) 
	    printf("working\n");
	else
	    printf("false\n");
      
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	printf("    using a simple file view...............");
    }	

    memset (inarr, 0, 12*sizeof(int)); 
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );

    MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );
    ret1 = MPI_File_read_shared ( file1, inarr, 4, MPI_INT, &status );
    ret2 = MPI_File_read_shared ( file1, &inarr[4], 4, MPI_INT, &status );
    ret3 = MPI_File_read_shared ( file1, &inarr[8], 4, MPI_INT, &status );
    MPI_File_close ( &file1);
    
    localret = 0;
    if ( ret1 != MPI_SUCCESS || ret2 != MPI_SUCCESS || ret3 != MPI_SUCCESS) {
	localret = 1;
    }
    /* verifying input data */
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    MPI_Gather (inarr, 12, MPI_INT, veriarr, 12, MPI_INT, root, comm);
    if ( rank == root ) {
	qsort ((int*)veriarr, 72, sizeof(int), intcompare);
	for ( i=0; i< 72; i++ ) {
	    if (veriarr[i] != i ) {
		globalret = 1;
#ifdef VERBOSE
		printf("Element %d is %d\n", i, veriarr[i]);
#endif
	    }
	}

	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	}

/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	printf("    using a file view w/ gaps..............");
    }	

    blength[0] = 6;
    displs[0]  = 0;
    dats[0]    = MPI_INT;
    blength[1] = 6;
    displs[1]  = 8*sizeof(int);
    dats[1]    = MPI_INT;

    MPI_Type_create_struct ( 2, blength, displs, dats, &tmp2);
    MPI_Type_commit ( &tmp2);
    MPI_Type_create_resized ( tmp2, 0, 16*sizeof(int), &fview2 );
    MPI_Type_commit ( &fview2);
    
    memset ( inarr, 0, 12*sizeof(int));
    memset ( veriarr, 0, 72*sizeof(int));
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );

    MPI_File_set_view ( file1, 0, MPI_INT, fview2, "native", MPI_INFO_NULL );
    localret = MPI_File_read_shared ( file1, inarr, 4, MPI_INT, MPI_STATUS_IGNORE );
    MPI_File_close ( &file1);
    MPI_Type_free ( &fview2);
    MPI_Type_free ( &tmp2);

    /* verifying input data */
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    MPI_Gather (inarr, 4, MPI_INT, veriarr, 4, MPI_INT, root, comm);

    if ( rank == root ) {
	qsort ((int*)veriarr, 24, sizeof(int), intcompare);
	for ( i=0, k=0; i<4 ; i++ ) {
            for ( j=0; j<6; j++ ) {
                if ( veriarr[k] != (i*8)+j) {
                    globalret = 1;
#ifdef VERBOSE
                    printf("Element %d is %d should be%d\n", k, veriarr[k], (i*8)+j);
#endif
                }
                k++;
	    }
	}

	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	}
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	printf("    using different communicators .........");
    }	

    memset (inarr, 0, 12*sizeof(int)); 
    MPI_File_open ( MPI_COMM_SELF, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );

    ret1 = MPI_File_read_shared ( file1, inarr, 4, MPI_INT, &status );
    ret2 = MPI_File_read_shared ( file1, &inarr[4], 4, MPI_INT, &status );
    ret3 = MPI_File_read_shared ( file1, &inarr[8], 4, MPI_INT, &status );
    MPI_File_close ( &file1);
    
    localret = 0;
    for (i=0; i< 12; i++ ) {
        if ( inarr[i] != i ) {
            localret = 1;
#ifdef VERBOSE
            printf("%d: data is %d should be %d\n", rank, inarr[i], i );
#endif
        }
    }
    
    if ( ret1 != MPI_SUCCESS || ret2 != MPI_SUCCESS || ret3 != MPI_SUCCESS) {
	localret = 1;
    }
    /* verifying input data */
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root ) {
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	}
        

/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	printf("Checking for MPI_File_read_ordered.........");
    }
    
    memset ( inarr, 0, 12*sizeof(int));
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );

    ret1 = MPI_File_read_ordered ( file1, inarr, 6, MPI_INT, &status );
    ret2 = MPI_File_read_ordered ( file1, &inarr[6], 6, MPI_INT, &status );
    MPI_File_close ( &file1 );

    localret = 1;
    if ( ret1 == MPI_SUCCESS && ret2 == MPI_SUCCESS ) {
	localret = 0;
    }
    for ( i=0; i<6; i++ ) {
	if ( inarr[i] != rank*6+i ) {
#ifdef VERBOSE
	    printf("[%d]:Element %d is %d should be %d\n", rank, i, inarr[i], (rank*6+i));
#endif
	    localret = 1;
	}
	if ( inarr[6+i] != rank*6 + 36 +i ) {
#ifdef VERBOSE
	    printf("[%d]:Element %d is %d should be %d\n", rank, 6+i, inarr[6+i], (rank*6+i+36));
#endif
	    localret = 1;
	}
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root ) {
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	}
	
    }
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
    if ( rank == root ) {
	printf("Checking for MPI_File_iread_shared.........");
    }

    memset (inarr, 0, 12*sizeof(int)); 
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );
    
    ret1 = MPI_File_iread_shared ( file1, inarr, 4, MPI_INT, &reqs[0] );
    ret2 = MPI_File_iread_shared ( file1, &inarr[4], 4, MPI_INT, &reqs[1] );
    ret3 = MPI_File_iread_shared ( file1, &inarr[8], 4, MPI_INT, &reqs[2] );

    MPI_Waitall ( 3, reqs, stats );
    MPI_File_close ( &file1);
    
    localret = 0;
    if ( ret1 != MPI_SUCCESS || ret2 != MPI_SUCCESS || ret3 != MPI_SUCCESS) {
	localret = 1;
    }
    /* verifying input data */
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    MPI_Gather (inarr, 12, MPI_INT, veriarr, 12, MPI_INT, root, comm);
    if ( rank == root ) {
	qsort ((int*)veriarr, 72, sizeof(int), intcompare);
	for ( i=0; i< 72; i++ ) {
	    if (veriarr[i] != i ) {
		globalret = 1;
#ifdef VERBOSE
		printf("Element %d is %d\n", i, veriarr[i]);
#endif
	    }
	}

	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	}
	
	printf("    verifying status field.................");
	MPI_Get_elements ( &stats[0], MPI_INT, &count );
	if ( count == 4 ) 
	    printf("working\n");
	else
	    printf("false\n");
       
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	printf("Checking for MPI_File_read_ordered_begin...");
    }
    
    memset ( inarr, 0, 12*sizeof(int));
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );

    ret1 = MPI_File_read_ordered_begin ( file1, inarr, 12, MPI_INT );
    ret2 = MPI_File_read_ordered_end ( file1, inarr, &status );
    MPI_File_close ( &file1 );

    localret = 1;
    if ( ret1 == MPI_SUCCESS && ret2 == MPI_SUCCESS ) {
	localret = 0;
    }
    for ( i=0; i<6; i++ ) {
	if ( inarr[i] != rank*12+i ) {
#ifdef VERBOSE
	    printf("[%d]:Element %d is %d should be %d\n", rank, i, inarr[i], (rank*6+i));
#endif
	    localret = 1;
	}
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root ) {
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	}
	unlink("readfile1.out");
    }

#ifndef GLOBAL
    MPI_Finalize ();
#endif

    return 0;
}

static int intcompare (const void *p, const void *q)
{
    int *a, *b;
  
    /* i.e. we cast and just compare the keys and then the original ranks.. */
    a = (int*)p;
    b = (int*)q;
    
    /* simple tests are those where the keys are different */
    if (*a < *b) {
        return (-1);
    }
    if (*a > *b) {
        return (1);
    }
    
    return ( 0 );
}




