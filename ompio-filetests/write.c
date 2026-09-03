/*
** This file test MPI_File_write. 
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mpi.h"
#include "common.h"

#ifdef GLOBAL
int write_test ( MPI_Comm comm, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
#endif

    int rank, size, i, j, ret, localret;
    MPI_Status status;
    MPI_File file1;
    int *bigarr=NULL;
    int writearr[6];
    int writearr2[18];
    int count;
    
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
	printf("Checking for MPI_File_write:\n");
	printf("    using single proc no fview.............");
    }


    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    if ( rank == root ) {
	for ( i=0; i<6; i++ ) {
	    writearr[i] = i;
	}

	ret = MPI_File_write ( file1, writearr, 6, MPI_INT, &status);
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
	
	if ( ret == MPI_SUCCESS && localret == 0 )
	    printf("working\n");
	else
	    printf("false\n");

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
	printf("    verifying status fields................");
	MPI_Get_elements ( &status, MPI_INT, &count );
	if ( count == 6 ) 
	    printf("working\n");
	else
	    printf("false\n");
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

	ret = MPI_File_write ( file1, writearr, 6, MPI_INT, &status);
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
	else
	    printf("false\n");
    }	
/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
#if 0 
	/* test disabled by default, since it takes a lot of time */
	printf("    writing by multiple processes..........");
    }

    /* Fourth test. Everybody write without fview using the append 
       mode. File is being opened and closed a lot of times 
       because of that. For rank zero we open in create mode,
       for all other in append mode. 
	
       Note, that this test can be very slow due to the sync()
       operations it requires for proper results. */

    for ( j=0; j<6; j++ ) {
	writearr[j] = rank*6+j;
    }
    
    MPI_File_open ( comm, "writefile2.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );
    if ( rank == 0 ) {
	ret = MPI_File_write ( file1, writearr, 6, MPI_INT, &status);
    }
    MPI_File_close (&file1);
    if ( rank == 0 ) sync();
    MPI_Barrier ( comm );

    for ( i=1; i< size; i++ ) {
	MPI_File_open ( comm, "writefile2.out", MPI_MODE_APPEND|MPI_MODE_WRONLY,
			MPI_INFO_NULL, &file1 );
	if ( rank == i ) {
	    ret = MPI_File_write ( file1, writearr, 6, MPI_INT, &status);
	}
	MPI_File_close (&file1);
        if ( rank == i ) sync ();
	MPI_Barrier ( comm );
    }
	    

    if ( rank == root ) {
	int fh, readarr[36];

	/* Verify content of the file using posix read operations */
	fh = open ("writefile2.out", O_RDONLY );
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
	else
	    printf("false\n");
    }
#endif 
    if ( rank == root ) {
	int fh;

       /* clean up the files generated in this test case. */
	unlink ("writefile1.out");
	unlink ("writefile2.out");

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
	MPI_File_write ( file1, bigarr, (12*1024*1024), MPI_INT, &status );
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
//#ifdef VERBOSE
		    printf("Element %d is %d\n", i, bigarr[i] );
//#endif
		    localret = 1;
		}
	    }
	    close (fh);
	}
	if ( localret == 0 ) 
	    printf("working\n");
	else 
	    printf("false\n");
	free ( bigarr );
	unlink ("bigfile.out");
    }
/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
    if ( rank == root ) {
	printf("Checking for MPI_File_write_at:\n");
	printf("    single write_at by each proc...........");
    }


    for ( j=0; j<6; j++ ) {
	writearr[j] = rank*6+j;
    }

    MPI_File_open ( comm, "writefile3.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    ret = MPI_File_write_at ( file1, rank*sizeof(int)*6, writearr, 6, MPI_INT, &status);
    MPI_File_close (&file1);

    if ( rank == root ) {
	int fh, readarr[36];

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
	else
	    printf("false\n");

       /* clean up the files generated in this test case. */
	unlink ("writefile3.out");
    }

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
    if ( rank == root ) {
	int fh, readarr[18];

	printf("    combining write and write_at...........");
	// In theory the explicit offset operations are not supposed to influence
	// the position of the implicit file pointer.

	for ( j=0; j<18; j++ ) {
	    writearr2[j] = j;
	}
	
	MPI_File_open ( MPI_COMM_SELF, "writefile4.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
			MPI_INFO_NULL, &file1 );
	
	MPI_File_write ( file1,  writearr2, 6, MPI_INT, &status);
	ret = MPI_File_write_at ( file1, 12*sizeof(int), &writearr2[12], 6, MPI_INT, &status);
	MPI_File_write ( file1,  &writearr2[6], 6, MPI_INT, &status);
	MPI_File_close ( &file1 );
	
	/* Verify content of the file using posix read operations */
	fh = open ("writefile4.out", O_RDONLY );
	if ( -1 == fh ) {
	    localret = 1;
	}
	else {
	    SL_read ( fh, readarr, 18 * sizeof(int));
	    localret = 0;
	    for ( i = 0; i< 18; i++ ) {
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
	else
	    printf("false\n");

       /* clean up the files generated in this test case. */
	unlink ("writefile4.out");
    }

#ifndef GLOBAL
    MPI_Finalize ();
#endif

    return 0;
}
