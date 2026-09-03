/*
** This file test MPI_File_write_all and their
** explicit offset versions
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

#define MAXLEN 33554432
#define BLOCK_SIZE 1000

int write_all_2D ( MPI_Comm comm, int root );

#ifdef GLOBAL
int write_all_test ( MPI_Comm comm, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
#endif

    int rank, size, i, ret, ret2, ret3, ret4, localret, globalret;
    MPI_Status status;
    MPI_File file1, file2;
    int inarr[12], count, *inarr2=NULL, displs2[2];
    MPI_Datatype fview1, fview2, fview3, fview4, dats[2];
    MPI_Datatype tmp1, tmp2, tmp3, tmp4;
    int blength[2];
    MPI_Aint displs[2];
    MPI_Request req[2];
    MPI_Status stats[2];
    void *data = NULL;
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

    /* create first the derived data types required for the file views. */
    blength[0] = 6;
    displs[0]  = rank*6*sizeof(int);
    dats[0]    = MPI_INT;
    MPI_Type_create_struct (1, blength, displs, dats, &tmp1);
    MPI_Type_commit (&tmp1 );
    MPI_Type_create_resized (tmp1, 0, 36*sizeof(int), &fview1);
    MPI_Type_commit (&fview1);

    /* Create a fileview where one process does not own any portion of the
       file. If you wander why this fview3 instead of fview2, don't worry
       about that - its just historic. */
    if ( rank == 0 ) {
	blength[0] = 0;
    }
    if ( rank == 1 ) {
	blength[0] = 12;
	displs[0] = 0;
    }
    
    MPI_Type_create_struct (1, blength, displs, dats, &tmp3);
    MPI_Type_commit (&tmp3 );
    MPI_Type_create_resized (tmp3, 0, 36*sizeof(int), &fview3);
    MPI_Type_commit (&fview3);

    blength[0] = 6;
    blength[1] = 6;
    displs[0]  = rank*6*sizeof(int);
    displs[1]  = 36*sizeof(int)+rank*6*sizeof(int);
    dats[0]    = MPI_INT;
    dats[1]    = MPI_INT;
    MPI_Type_create_struct ( 2, blength, displs, dats, &tmp2);
    MPI_Type_commit (&tmp2 );
    MPI_Type_create_resized ( tmp2, 0, 72*sizeof(int), &fview2 );
    MPI_Type_commit ( &fview2);


    /* initialize input data */
    for ( i=0; i<6; i++ ) {
	inarr[i] = rank*6+i;
	inarr[6+i] = rank*6 + 36 +i;
    }

    if ( rank == root ) {
	printf("Checking for MPI_File_write_all:\n");
	printf("    using the file view repetitivly........");
    }

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );

    ret = MPI_File_write_all ( file1, inarr, 12, MPI_INT, &status );
    MPI_File_close ( &file1);
    
    if ( ret == MPI_SUCCESS ) {
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
	if ( count == 12 ) 
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
	
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	unlink("writefile1.out");
	printf("    write less than the file view..........");
    }

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );
    MPI_File_write_all ( file1, &inarr[0], 3, MPI_INT, &status );
    MPI_File_write_all ( file1, &inarr[3], 3, MPI_INT, &status );
    MPI_File_write_all ( file1, &inarr[6], 3, MPI_INT, &status );
    MPI_File_write_all ( file1, &inarr[9], 3, MPI_INT, &status );
    MPI_File_close ( &file1);

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
	    for ( i=0; i< 72; i++ ) {
		if (veriarr[i] != i ) {
		    localret = 1;
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, veriarr[i]);
#endif
		}
	    }
	}
	if ( ret == MPI_SUCCESS && localret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
	
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	unlink("writefile1.out");
	printf("    writing a file view with gaps..........");
    }

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview2, "native", MPI_INFO_NULL );

    MPI_File_write_all ( file1, inarr, 12, MPI_INT, &status );
    MPI_File_close ( &file1);
    
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
	    for ( i=0; i< 72; i++ ) {
		if (veriarr[i] != i ) {
		    localret = 1;
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, veriarr[i]);
#endif
		}
	    }
	}
	if ( ret == MPI_SUCCESS && localret == 0 ) {
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
	printf("    using file view w/ displacement........");
    }

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    if ( rank == 0 ) {
	int tarr[2];
	tarr[0] = -1;
	tarr[1] = -2;
	MPI_File_write ( file1, tarr, 2, MPI_INT, &status);
    }

    MPI_Barrier ( comm );
    ret = MPI_File_set_view ( file1, 8, MPI_INT, fview1, "native", MPI_INFO_NULL );

    MPI_File_write_all ( file1, inarr, 12, MPI_INT, &status );
    MPI_File_close ( &file1);
    
    if ( rank == 0 ) {
	int fh, veriarr[72];
	
	fh = open ("writefile1.out", O_RDONLY );
	if ( -1 == fh ) {
	    printf("Could not open file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    localret = 0;
	    SL_read ( fh, veriarr, 2 * sizeof(int));
	    if ( veriarr[0] != -1 || veriarr[1] != -2 ) {
		localret = 1;
#ifdef VERBOSE
		    printf("Header Element should be -1 and -2, are %d, %d\n", 
			   veriarr[0], veriarr[1]);
#endif
	    }
	    SL_read ( fh, veriarr, 72 * sizeof(int));
	    for ( i=0; i< 72; i++ ) {
		if (veriarr[i] != i ) {
		    localret = 1;
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, veriarr[i]);
#endif
		}
	    }
	}
	if ( ret == MPI_SUCCESS && localret == 0 ) {
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
	printf("    file view w/o elems for rank 0.........");
    }
    MPI_Barrier ( comm );
    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file2 );

    ret = MPI_File_set_view ( file2, 0, MPI_INT, fview3, "native", MPI_INFO_NULL );

    if ( rank == 0 ) {
	MPI_File_write_all ( file2, inarr, 0, MPI_INT, &status );
    }
    else if ( rank == 1 ) {
	int inarr2[12];
	for ( i=0; i<12; i++ ) {
	    inarr2[i]=i;
	}
	MPI_File_write_all ( file2, inarr2, 12, MPI_INT, &status );
    }
    else {
	MPI_File_write_all ( file2, inarr, 6, MPI_INT, &status );
    }

    MPI_File_close ( &file2);
    
    if ( rank == 0 ) {
	int fh, veriarr[36];
	
	fh = open ("writefile1.out", O_RDONLY );
	if ( -1 == fh ) {
	    printf("Could not open file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    localret = 0;
	    SL_read ( fh, veriarr, 36 * sizeof(int));
	    for ( i=0; i<36; i++ ) {
		if (veriarr[i] != i ) {
		    localret = 1;
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, veriarr[i]);
#endif
		}
	    }
	}
	if ( ret == MPI_SUCCESS && localret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
	unlink("writefile1.out");	
	close ( fh );
    }
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
    if ( rank == root ) {
	printf("    writing a large file...................");
    }

   /* create first the derived data types required for the file views. */
    blength[0] = MAXLEN;
    displs2[0]  = rank*MAXLEN;
    MPI_Type_indexed (1, blength, displs2, MPI_INT, &tmp4);
    MPI_Type_commit (&tmp4 );
    MPI_Type_create_resized (tmp4, 0, MAXLEN*size*sizeof(int), &fview4);
    MPI_Type_commit (&fview4);

    /* initialize input data */
    inarr2 = (int *) malloc ( MAXLEN * sizeof(int));
    for (i=0; i<MAXLEN; i++ ) {
        inarr2[i] = rank*MAXLEN + i;
    }

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
                    MPI_INFO_NULL, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview4, "native", MPI_INFO_NULL );

    localret = MPI_File_write_all ( file1, inarr2, MAXLEN, MPI_INT, &status );
    MPI_File_close ( &file1);
    free ( inarr2);

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    if ( rank == root ) {
        int fh, *veriarr;
        veriarr = (int *) calloc ( MAXLEN, sizeof(int)*size );

        fh = open ("writefile1.out", O_RDONLY );
        if ( -1 == fh ) {
            printf("Could not open file \n");
           MPI_Abort ( comm , -1 );
        }
        else {
            SL_read ( fh, veriarr, MAXLEN*size*sizeof(int));
            for ( i=0; i< MAXLEN*size; i++ ) {
                if (veriarr[i] != i ) {
                    globalret = 1;
#ifdef VERBOSE
                    printf("Element %d: expected %d got %d", i, i, veriarr[i]);
#endif
                }
            }
            close (fh);
        }
        unlink("writefile1.out");
        free (veriarr);

	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
    }

    MPI_Type_free ( &fview4 );
    MPI_Type_free ( &tmp4 );

/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
    /* Another test case based on a user report */
    total += write_all_2D ( comm, root);

/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
    if ( rank == root ) {
	printf("Checking for MPI_File_write_at_all:\n");
	printf("    using no file view ....................");    
    }
    MPI_Barrier ( comm );
    MPI_File_open ( comm, "writefile2.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    ret = MPI_File_write_at_all ( file1, displs[0], inarr, 6, MPI_INT, &status );
    ret2 = MPI_File_write_at_all ( file1, displs[1], &inarr[6], 6, MPI_INT, &status );
    MPI_File_close ( &file1 );

    localret = 1;
    if ( ret == MPI_SUCCESS && ret2 == MPI_SUCCESS ) {
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
	printf("    using file view w/ displacement........");    

    }

    MPI_File_open ( comm, "writefile3.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    if ( rank == 0 ) {
	int tarr[2];
	tarr[0] = -1;
	tarr[1] = -2;
	MPI_File_write ( file1, tarr, 2, MPI_INT, &status);
    }

    MPI_Barrier ( comm );
    ret = MPI_File_set_view ( file1, 8, MPI_INT, fview1, "native", MPI_INFO_NULL );

    MPI_File_write_at_all ( file1, 0, inarr, 6, MPI_INT, &status );
    MPI_File_write_at_all ( file1, 6, &inarr[6], 6, MPI_INT, &status );
    MPI_File_close ( &file1);
    if ( rank == 0 ) {
	int fh, veriarr[72];
	
	fh = open ("writefile3.out", O_RDONLY );
	if ( -1 == fh ) {
	    printf("Could not open file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    localret = 0;
	    SL_read ( fh, (char *) veriarr, 2 * sizeof(int));
	    if ( veriarr[0] != -1 || veriarr[1] != -2 ) {
		localret = 1;
#ifdef VERBOSE
		    printf("Header Element should be -1 and -2, are %d, %d\n", 
			   veriarr[0], veriarr[1]);
#endif
	    }
	    SL_read ( fh, (char *) veriarr, 72 * sizeof(int));
	    for ( i=0; i< 72; i++ ) {
		if (veriarr[i] != i ) {
		    localret = 1;
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, veriarr[i]);
#endif
		}
	    }
	}
	if ( ret == MPI_SUCCESS && localret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
	unlink("writefile3.out");	
    }

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
    if ( rank == root ) {
	// This next test case is based on an HDF5 test case 
	printf("    writing zero bytes on some procs.......");
    }

    data = malloc(BLOCK_SIZE);
    memset(data, 0, BLOCK_SIZE);

    if(root == rank) {
        count = BLOCK_SIZE;
    }
    else {
        count = 0;
    }

    MPI_File_open (comm, "writefile1.out", MPI_MODE_RDWR | MPI_MODE_CREATE,
		   MPI_INFO_NULL, &file1); 

    ret =  MPI_File_write_at_all (file1, 
				  2144, 
				  data, 
				  count, 
				  MPI_BYTE, 
				  MPI_STATUS_IGNORE);

    MPI_File_close(&file1);
    free(data);

    if ( rank == root ) {
        int fh;
	
	fh = open ("writefile1.out", O_RDONLY );
	if ( -1 == fh ) {
	    printf("Could not open file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    off_t size;
	    size = lseek(fh, 0, SEEK_END); 
	    if ( size == 3144 ) {
		printf ("working\n");
	    }
	    else {
		printf ("false %d\n", (int) size);
	    }
	    /* clean up the files generated in this test case. */
	    unlink ("writefile1.out");
	}
    }

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
    if ( rank == root ) {
	printf("Checking for MPI_File_write_all_begin......");
    }

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );

    ret = MPI_File_write_all_begin ( file1, inarr, 12, MPI_INT );
    ret2 = MPI_File_write_all_end ( file1, inarr, &status );
    MPI_File_close ( &file1);
    
    if ( ret == MPI_SUCCESS && ret2 == MPI_SUCCESS ) {
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
	if ( count == 12 ) 
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
	
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	unlink("writefile1.out");
	printf("Checking for MPI_File_write_at_all_begin...");
    }
    MPI_Barrier ( comm );
    MPI_File_open ( comm, "writefile2.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    ret = MPI_File_write_at_all_begin ( file1, displs[0], inarr, 6, MPI_INT);
    ret2 = MPI_File_write_at_all_end ( file1, inarr, &status );
    ret3 = MPI_File_write_at_all_begin ( file1, displs[1], &inarr[6], 6, MPI_INT );
    ret4 = MPI_File_write_at_all_end ( file1, &inarr[6], &status );
    MPI_File_close ( &file1 );

    localret = 1;
    if ( ret == MPI_SUCCESS && ret2 == MPI_SUCCESS && ret3 == MPI_SUCCESS && ret4 == MPI_SUCCESS) {
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
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
    if ( rank == root ) {
	printf("Checking for MPI_File_iwrite_all...........");
    }

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );

    ret = MPI_File_iwrite_all ( file1, inarr, 12, MPI_INT, &req[0] );
    ret2 = MPI_Wait ( &req[0], &status );
    MPI_File_close ( &file1);
    
    if ( ret == MPI_SUCCESS && ret2 == MPI_SUCCESS ) {
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
	if ( count == 12 ) 
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
	
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
	unlink("writefile1.out");
	printf("Checking for MPI_File_iwrite_at_all........");
    }
    MPI_Barrier ( comm );
    MPI_File_open ( comm, "writefile2.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );

    ret  = MPI_File_iwrite_at_all ( file1, displs[0], inarr, 6, MPI_INT, &req[0]);
    ret2 = MPI_File_iwrite_at_all ( file1, displs[1], &inarr[6], 6, MPI_INT, &req[1] );
    ret3 = MPI_Waitall ( 2, req, stats );
    MPI_File_close ( &file1 );

    localret = 1;
    if ( ret == MPI_SUCCESS && ret2 == MPI_SUCCESS && ret3 == MPI_SUCCESS ) {
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

    


    MPI_Type_free ( &fview1 );
    MPI_Type_free ( &fview2 );
    MPI_Type_free ( &fview3 );
    MPI_Type_free ( &tmp1 );
    MPI_Type_free ( &tmp2 );
    MPI_Type_free ( &tmp3 );

#ifndef GLOBAL
    MPI_Finalize ();
#endif

    return total;
}