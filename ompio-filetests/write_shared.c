/*
** This file test MPI_File_write_shared and MPI_File_write_ordered
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"
#include "common.h"

static int intcompare(const void *, const void *);


#ifdef GLOBAL
int write_shared_test ( MPI_Comm comm, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
#endif

    int rank, size, i, ret1, ret2, ret3, localret, globalret;
    MPI_Status status, stats[3];
    MPI_Request reqs[3];    
    MPI_File file1;
    int inarr[12], count;
    int total = 0;

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

    /* initialize input data */
    for ( i=0; i<6; i++ ) {
	inarr[i] = rank*6+i;
	inarr[6+i] = rank*6 + 36 +i;
    }

    if ( rank == root ) {
	printf("Checking for MPI_File_write_shared.........");
    }

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    ret1 = MPI_File_write_shared ( file1, inarr, 4, MPI_INT, &status );
    ret2 = MPI_File_write_shared ( file1, &inarr[4], 4, MPI_INT, &status );
    ret3 = MPI_File_write_shared ( file1, &inarr[8], 4, MPI_INT, &status );

    MPI_File_close ( &file1);
    
    if ( ret1 == MPI_SUCCESS  && ret2 == MPI_SUCCESS && ret3 == MPI_SUCCESS) {
	localret = 0;
    }
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == 0 ) {
	int fh, veriarr[72];
	
	fh = open ("writefile1.out", O_RDONLY );
	if ( -1 == fh ) {
	    printf("Could not open file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    SL_read ( fh, veriarr, 72 * sizeof(int));
	    localret = 0;
	    qsort ((int*)veriarr, 72, sizeof(int), intcompare);
	    for ( i=0; i< 72; i++ ) {
		if (veriarr[i] != i ) {
		    globalret = 1;
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, veriarr[i]);
#endif
		}
	    }
	}
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	printf("    verifying status field.................");
	MPI_Get_elements ( &status, MPI_INT, &count );
	if ( count == 4 ) 
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}

	unlink("writefile1.out");
    }

/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
    MPI_Barrier ( comm );
    if ( rank == root ) {
	printf("    using a simple file view...............");
    }

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );
    ret1 = MPI_File_write_shared ( file1, inarr, 4, MPI_INT, &status );
    ret2 = MPI_File_write_shared ( file1, &inarr[4], 4, MPI_INT, &status );
    ret3 = MPI_File_write_shared ( file1, &inarr[8], 4, MPI_INT, &status );

    MPI_File_close ( &file1);
    
    if ( ret1 == MPI_SUCCESS  && ret2 == MPI_SUCCESS && ret3 == MPI_SUCCESS) {
	localret = 0;
    }
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == 0 ) {
	int fh, veriarr[72];
	
	fh = open ("writefile1.out", O_RDONLY );
	if ( -1 == fh ) {
	    printf("Could not open file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    SL_read ( fh, veriarr, 72 * sizeof(int));
	    localret = 0;
	    qsort ((int*)veriarr, 72, sizeof(int), intcompare);
	    for ( i=0; i< 72; i++ ) {
		if (veriarr[i] != i ) {
		    globalret = 1;
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, veriarr[i]);
#endif
		}
	    }
	}
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
	unlink("writefile1.out");
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	

	printf("    opening file in append mode............");
    }
    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    if ( rank == root ) {
	ret1 = MPI_File_write ( file1, inarr, 6, MPI_INT, &status);
    }

    MPI_File_close ( &file1 );


    MPI_File_open ( comm, "writefile1.out", MPI_MODE_APPEND|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    if ( rank == root ) {
	ret1 = MPI_File_write_shared ( file1, inarr, 6, MPI_INT, &status);
    }
    
    MPI_File_close ( &file1 );

    if ( rank == root ) {
	int j, fh,  readarr[12];

	/* Verify content of the file using posix read operations */
	fh = open ("writefile1.out", O_RDONLY );
	if ( -1 == fh ) {
	    localret = 1;
	}
	else {
	    SL_read ( fh, readarr, 12 * sizeof(int));
	    localret = 0;
            for ( j=0; j<2; j++ ) {
                for ( i = 0; i< 6; i++ ) {
                    if ( readarr[6*j+i] != i ) {
#ifdef VERBOSE
                        printf("Element %d is %d\n", i, readarr[i] );
#endif
                            localret = 1;
                    }
                }
            }
	    close (fh);
	}
	
	if ( ret1 == MPI_SUCCESS && localret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
        
	unlink("writefile1.out");

/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	printf("Checking for MPI_File_write_ordered........");
    }

    MPI_Barrier ( comm );
    MPI_File_open ( comm, "writefile2.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    ret1 = MPI_File_write_ordered ( file1, inarr, 6, MPI_INT, &status );
    ret2 = MPI_File_write_ordered ( file1, &inarr[6], 6, MPI_INT, &status );
    MPI_File_close ( &file1 );

    localret = 1;
    if ( ret1 == MPI_SUCCESS && ret2 == MPI_SUCCESS ) {
	localret = 0;
    }
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == 0 ) {
	int fh, veriarr[72];
	
	fh = open ("writefile2.out", O_RDONLY );
	if ( -1 == fh ) {
	    printf("Could not open file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    SL_read ( fh, (char *) veriarr, 72 * sizeof(int));
	    localret = 0;
	    qsort ((int*)veriarr, 72, sizeof(int), intcompare);
	    for ( i=0; i< 72; i++ ) {
		if (veriarr[i] != i ) {
		    globalret = 1;
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, veriarr[i]);
#endif
		}
	    }
	}
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
	
	unlink("writefile2.out");
    }
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
    if ( rank == root ) {
	printf("Checking for MPI_File_iwrite_shared........");
    }

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    ret1 = MPI_File_iwrite_shared ( file1, inarr, 4, MPI_INT, &reqs[0] );
    ret2 = MPI_File_iwrite_shared ( file1, &inarr[4], 4, MPI_INT, &reqs[1]);
    ret3 = MPI_File_iwrite_shared ( file1, &inarr[8], 4, MPI_INT, &reqs[2] );

    MPI_Waitall ( 3, reqs, stats);
    MPI_File_close ( &file1);
    
    if ( ret1 == MPI_SUCCESS  && ret2 == MPI_SUCCESS && ret3 == MPI_SUCCESS) {
	localret = 0;
    }
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == 0 ) {
	int fh, veriarr[72];
	
	fh = open ("writefile1.out", O_RDONLY );
	if ( -1 == fh ) {
	    printf("Could not open file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    SL_read ( fh, veriarr, 72 * sizeof(int));
	    localret = 0;
	    qsort ((int*)veriarr, 72, sizeof(int), intcompare);
	    for ( i=0; i< 72; i++ ) {
		if (veriarr[i] != i ) {
		    globalret = 1;
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, veriarr[i]);
#endif
		}
	    }
	}
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	printf("    verifying status field.................");
	MPI_Get_elements ( &stats[0], MPI_INT, &count );
	if ( count == 4 ) 
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}

	unlink("writefile1.out");
    }

/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
    if ( rank == root ) {
	printf("Checking for MPI_File_write_ordered_begin..");
    }

    MPI_Barrier ( comm );
    MPI_File_open ( comm, "writefile2.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    ret1 = MPI_File_write_ordered_begin ( file1, inarr, 12, MPI_INT );
    ret2 = MPI_File_write_ordered_end ( file1, inarr, &status );
    MPI_File_close ( &file1 );

    localret = 1;
    if ( ret1 == MPI_SUCCESS && ret2 == MPI_SUCCESS ) {
	localret = 0;
    }
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == 0 ) {
	int fh, veriarr[72];
	
	fh = open ("writefile2.out", O_RDONLY );
	if ( -1 == fh ) {
	    printf("Could not open file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    SL_read ( fh, (char *) veriarr, 72 * sizeof(int));
	    localret = 0;
	    qsort ((int*)veriarr, 72, sizeof(int), intcompare);
	    for ( i=0; i< 72; i++ ) {
		if (veriarr[i] != i ) {
		    globalret = 1;
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, veriarr[i]);
#endif
		}
	    }
	}
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
	
	unlink("writefile2.out");
    }

#ifndef GLOBAL
    MPI_Finalize ();
#endif

    return total;
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
