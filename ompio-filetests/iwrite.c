/*
** This file test MPI_File_iwrite. 
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mpi.h"
#include "common.h"

#define MAX_NUM_TESTS 1000

#ifdef GLOBAL
int iwrite_test ( MPI_Comm comm, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
#endif

    int rank, size, i, j, ret, ret2, localret;
    MPI_File file1;
    MPI_Request req;
    MPI_Status stat;
    int *bigarr=NULL;
    int writearr[6];
    int total = 0;

    
#ifndef GLOBAL
    MPI_Init ( &argc, &argv );
    if (argc > 1 ) sleep (20);
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
	printf("Checking for MPI_File_iwrite:\n");
	printf("    using single proc no fview.............");
    }


    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    if ( rank == root ) {
	for ( i=0; i<6; i++ ) {
	    writearr[i] = i;
	}

	ret = MPI_File_iwrite ( file1, writearr, 6, MPI_INT, &req);
	ret2 = MPI_Wait ( &req, &stat);
    }

    MPI_File_close ( &file1 );

    if ( rank == root ) {
	int fh, count, readarr[6];

	/* Verify content of the file using posix read operations */
	fh = open ("writefile1.out", O_RDONLY );
	if ( -1 == fh ) {
	    localret = 1;
	}
	else {
	    SL_read ( fh, readarr, 6 * sizeof(int));
	    localret = 0;
	    for ( i = 0; i< 6; i++ ) {
		if ( readarr[i] != i ) {
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, readarr[i] );
#endif
		    localret = 1;
		}
	    }
	    close (fh);
	}
	
	if ( ret == MPI_SUCCESS && ret2 == MPI_SUCCESS && localret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
	printf("    verifying status fields................");
	MPI_Get_elements ( &stat, MPI_INT, &count );
	if ( count == 6 ) 
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
	printf("    opening file in append mode............");
    }

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_APPEND|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    if ( rank == root ) {
	for ( j=0; j<6; j++ ) {
	    writearr[j] = 6+j;
	}

	ret = MPI_File_iwrite ( file1, writearr, 6, MPI_INT, &req);
	MPI_Wait ( &req, &stat );
    }
    
    MPI_File_close ( &file1 );

    if ( rank == root ) {
	int fh,  readarr[12];

	/* Verify content of the file using posix read operations */
	fh = open ("writefile1.out", O_RDONLY );
	if ( -1 == fh ) {
	    localret = 1;
	}
	else {
	    SL_read ( fh, readarr, 12 * sizeof(int));
	    localret = 0;
	    for ( i = 0; i< 12; i++ ) {
		if ( readarr[i] != i ) {
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, readarr[i] );
#endif
		    localret = 1;
		}
	    }
	    close (fh);
	}
	
	if ( ret == MPI_SUCCESS && localret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
    }	
/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
    if ( rank == root ) {
	int fh;

       /* clean up the files generated in this test case. */
	unlink ("writefile1.out");

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/

	printf("    writing a large file...................");
	bigarr = (int *) malloc ( 12 * 1024 * 1024 * sizeof(int));
	if ( NULL == bigarr)  {
	    printf("Could not allocate memory\n");
	    MPI_Abort(comm, 1);
	}
	for ( i=0; i<(12*1024*1024); i++ ) {
	    bigarr[i] = i;
	}
	MPI_File_open ( MPI_COMM_SELF, "bigfile.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
			MPI_INFO_NULL, &file1);
	MPI_File_iwrite ( file1, bigarr, (12*1024*1024), MPI_INT, &req );
	MPI_Wait ( &req, &stat);
	MPI_File_close ( &file1 );

	/* Verify content of the file using posix read operations */
	fh = open ("bigfile.out", O_RDONLY );
	if ( -1 == fh ) {
	    localret = 1;
	}
	else {
	    memset ( bigarr, 0, 12*1024*1024*sizeof(int) );
	    SL_read ( fh, bigarr, 12*1024*1024*sizeof(int));
	    localret = 0;
	    for ( i = 0; i< (12*1024*1024); i++ ) {
		if ( bigarr[i] != i ) {
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, bigarr[i] );
#endif
		    localret = 1;
		}
	    }
	    close (fh);
	}
	if ( localret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
	free ( bigarr );
	unlink ("bigfile.out");
/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
	printf("    writing zero bytes.....................");
	MPI_File_open ( MPI_COMM_SELF, "tinyfile.out", MPI_MODE_CREATE|MPI_MODE_WRONLY|MPI_MODE_DELETE_ON_CLOSE,
			MPI_INFO_NULL, &file1);
	ret = MPI_File_iwrite ( file1, NULL, 0, MPI_INT, &req );
	ret2 = MPI_Wait ( &req, &stat);
	MPI_File_close ( &file1 );
        
        if ( MPI_SUCCESS == ret && MPI_SUCCESS == ret2 ) {
            printf("working\n");
        }
        else {
            printf("false\n");
            total++;
        }
/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
	printf("    creating many non-blocking writes......");

        MPI_Request *reqs;
        reqs = (MPI_Request *) malloc ( MAX_NUM_TESTS * sizeof (MPI_Request) );

        localret=0;
        for ( j=0; j<6; j++ ) {
            writearr[j] = rank*6+j;
        }

        MPI_File_open ( MPI_COMM_SELF, "writefile3.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
                        MPI_INFO_NULL, &file1 );

        for (i=0; i< MAX_NUM_TESTS; i++ ) {
            ret = MPI_File_iwrite( file1, writearr, 6, MPI_INT, &reqs[i]);
            if ( MPI_SUCCESS != ret ) {
                localret = 1;
            }
        }
        ret = MPI_Waitall ( MAX_NUM_TESTS, reqs, MPI_STATUS_IGNORE );
        if ( MPI_SUCCESS != ret ) {
            localret = 1;
        }
        MPI_File_close (&file1);
	if ( localret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
	free ( reqs );
	unlink ("writefile3.out");

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
	printf("Checking for MPI_File_iwrite_at............");
    }


    for ( j=0; j<6; j++ ) {
	writearr[j] = rank*6+j;
    }

    MPI_File_open ( comm, "writefile3.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    ret = MPI_File_iwrite_at ( file1, rank*sizeof(int)*6, writearr, 6, MPI_INT, &req);
    MPI_Wait ( &req, &stat );
    MPI_File_close (&file1);

    if ( rank == root ) {
	int fh, readarr[36], count;

	/* Verify content of the file using posix read operations */
	fh = open ("writefile3.out", O_RDONLY );
	if ( -1 == fh ) {
	    localret = 1;
	}
	else {
	    SL_read ( fh, readarr, 36 * sizeof(int));
	    localret = 0;
	    for ( i = 0; i< 36; i++ ) {
		if ( readarr[i] != i ) {
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, readarr[i] );
#endif
		    localret = 1;
		}
	    }
	    close (fh);
	}
	
	if ( ret == MPI_SUCCESS && localret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}


	printf("    verifying status fields................");
	MPI_Get_elements ( &stat, MPI_INT, &count );
	if ( count == 6 ) 
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}

       /* clean up the files generated in this test case. */
	unlink ("writefile3.out");
    }
#ifndef GLOBAL
    MPI_Finalize ();
#endif

    return total;
}