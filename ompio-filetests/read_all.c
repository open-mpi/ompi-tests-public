/*
** This file test MPI_File_read_all and its
** explicit offset version
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

#define MAXLEN 33554432


#ifdef GLOBAL
int read_all_test ( MPI_Comm comm, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
#endif

    int rank, size, i, ret, ret2, ret3, ret4, localret, globalret;
    MPI_Status status;
    MPI_File file1;
    int inarr[12], count;
    MPI_Datatype fview1, fview2, dats[2],tmp4,fview4;
    MPI_Datatype tmp1, tmp2;
    int blength[2], *inarr2;
    MPI_Aint displs[2];
    int displs2[2];
    MPI_Request req[2];
    MPI_Status stats[2];
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
    MPI_Type_create_struct ( 1, blength, displs, dats, &tmp1);
    MPI_Type_commit ( &tmp1);
    MPI_Type_create_resized (tmp1, 0, 36*sizeof(int), &fview1);
    MPI_Type_commit (&fview1);


    blength[0]= 6;
    blength[1]= 6;
    displs[0] = rank*6*sizeof(int);
    displs[1] = 36*sizeof(int)+rank*6*sizeof(int);
    dats[0]   = MPI_INT;
    dats[1]   = MPI_INT;
    MPI_Type_create_struct ( 2, blength, displs, dats, &tmp2);
    MPI_Type_commit ( &tmp2);
    MPI_Type_create_resized ( tmp2, 0, 72*sizeof(int), &fview2 );
    MPI_Type_commit ( &fview2);


    /* create the input file */
    if ( rank == root ) {
	int fh, old_mask, perm;
	int veriarr[72];

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
	    SL_write ( fh, veriarr, 72 * sizeof(int));
	    close (fh);
	}
    }

    MPI_Barrier ( comm );
    if ( rank == root ) {
	printf("Checking for MPI_File_read_all:\n");
	printf("    using the file view repetitivly........");
    }

    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );
    
    ret = MPI_File_read_all ( file1, inarr, 12, MPI_INT, &status );
    MPI_File_close ( &file1);
    
    localret = 0;
    if ( ret != MPI_SUCCESS ) {
	localret = 1;
    }
    /* verifying input data */
    for ( i=0; i<6; i++ ) {
	if ( inarr[i] != rank*6+i ) 
	    localret = 1;
	if ( inarr[6+i] != rank*6 + 36 +i ) 
	    localret = 1;
    }
    

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root ) {
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
	printf("    reading less than the file view........");
    }
    memset ( inarr, 0, 12*sizeof(int));
	     
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );
    MPI_File_read_all ( file1, &inarr[0], 3, MPI_INT, &status );
    MPI_File_read_all ( file1, &inarr[3], 3, MPI_INT, &status );
    MPI_File_read_all ( file1, &inarr[6], 3, MPI_INT, &status );
    MPI_File_read_all ( file1, &inarr[9], 3, MPI_INT, &status );
    MPI_File_close ( &file1);

    /* verifying input data */
    localret = 0;
    for ( i=0; i<6; i++ ) {
	if ( inarr[i] != rank*6+i )  
	    localret = 1;
	if ( inarr[6+i] != rank*6 + 36 +i ) 
	    localret = 1;
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    if ( rank == root ) {
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
	printf("    reading a file view with gaps..........");
    }
    
    memset ( inarr, 0, 12*sizeof(int));
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview2, "native", MPI_INFO_NULL );

    MPI_File_read_all ( file1, inarr, 12, MPI_INT, &status );
    MPI_File_close ( &file1);
    
    /* verifying input data */
    localret = 0;
    for ( i=0; i<6; i++ ) {
	if ( inarr[i] != rank*6+i ) 
	    localret = 1;
	if ( inarr[6+i] != rank*6 + 36 +i ) 
	    localret = 1;
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    if ( rank == root ) {
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
    }

/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
    if ( rank == root ) {    
	printf("    reading a large file...................");
    }
   /* create first the derived data types required for the file views. */
    blength[0] = MAXLEN;
    displs2[0]  = rank*MAXLEN;
    MPI_Type_indexed (1, blength, displs2, MPI_INT, &tmp4);
    MPI_Type_commit (&tmp4 );
    MPI_Type_create_resized (tmp4, 0, MAXLEN*size*sizeof(int), &fview4);
    MPI_Type_commit (&fview4);

    /* initialize input data */
    inarr2 = (int *) calloc ( MAXLEN, sizeof(int));
    if ( rank == root ) {
        int fh, *veriarr, old_mask, perm;
        veriarr = (int *) calloc ( MAXLEN, sizeof(int)*size );
        for ( i=0; i<MAXLEN*size;i++) {
            veriarr[i]=i;
        }
        
	old_mask = umask(022);
	umask (old_mask);
	perm = old_mask^0666;

	fh = open ("readfile2.out", O_CREAT|O_WRONLY, perm );
	if ( -1 == fh ) {
	    printf("Could not create input file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    SL_write ( fh, veriarr, MAXLEN * size * sizeof(int));
	}
        free ( veriarr);
        close (fh);
    }
    MPI_File_open ( comm, "readfile2.out", MPI_MODE_RDONLY,
                    MPI_INFO_NULL, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview4, "native", MPI_INFO_NULL );

    localret = MPI_File_read_all ( file1, inarr2, MAXLEN, MPI_INT, &status );
    MPI_File_close ( &file1);

    for (i=0; i<MAXLEN; i++ ) {
        if ( inarr2[i] != rank*MAXLEN + i ){
            localret=1;
        }
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    if ( rank == root ) {
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
        unlink("readfile2.out");
    }

    MPI_Type_free ( &fview4 );
    MPI_Type_free ( &tmp4 );
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
    /* Create a fileview where one process does not own any portion of the
    ** file. */
    blength[0] = 6;
    displs[0]  = rank*6*sizeof(int);
    dats[0]    = MPI_INT;
    if (rank == 0) {
        blength[0] = 0;
    }
    if (rank == 1) {
        blength[0] = 12;
        displs[0]  = 0;
    }

    MPI_Type_create_struct (1, blength, displs, dats, &tmp4);
    MPI_Type_commit (&tmp4);
    MPI_Type_create_resized (tmp4, 0, 36*sizeof(int), &fview4);
    MPI_Type_commit (&fview4);

    if (rank == 0) {
	printf("    file view w/o elems for rank 0.........");
    }
    MPI_Barrier (comm);
    MPI_File_open (comm, "readfile1.out", MPI_MODE_RDONLY,
		   MPI_INFO_NULL, &file1);

    ret = MPI_File_set_view (file1, 0, MPI_INT, fview4, "native", MPI_INFO_NULL );
    if (rank == 0) {
        MPI_File_read_all (file1, inarr, 0, MPI_INT, &status);
    }
    else if (rank == 1) {
	inarr2 = (int *) calloc (12, sizeof(int));
        MPI_File_read_all (file1, inarr2, 12, MPI_INT, &status);
    }
    else {
        MPI_File_read_all (file1, inarr, 6, MPI_INT, &status);
    }
    MPI_File_close (&file1);

    /* verifying data */
    localret = 0;
    if (rank > 1) {
	for (i=0; i<6; i++) {
	    if ( inarr[i] != rank*6+i)
		localret = 1;
	}
    } else if (rank == 1) {
	for (i=0; i<12; i++) {
	    if (inarr2[i] != i)
		localret = 1;
	}
	free (inarr2);
    }

    MPI_Reduce (&localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    if (rank == root) {
	if (globalret == 0) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
    }
    MPI_Type_free (&fview4);
    MPI_Type_free (&tmp4);

    /* Restore displs[0] for subsequent tests */
    displs[0]  = rank*6*sizeof(int);
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
    if ( rank == root ) {    
	printf("Checking for MPI_File_read_at_all:\n");
	printf("    without file view......................");
    }
    
    memset ( inarr, 0, 12*sizeof(int));
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );

    ret = MPI_File_read_at_all ( file1, displs[0], inarr, 6, MPI_INT, &status );
    ret2 = MPI_File_read_at_all ( file1, displs[1], &inarr[6], 6, MPI_INT, &status );
    MPI_File_close ( &file1 );

    localret = 1;
    if ( ret == MPI_SUCCESS && ret2 == MPI_SUCCESS ) {
	localret = 0;
    }
    for ( i=0; i<6; i++ ) {
	if ( inarr[i] != rank*6+i ) 
	    localret = 1;
	if ( inarr[6+i] != rank*6 + 36 +i ) 
	    localret = 1;
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root ) {
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
	printf("    using file view w/ displacement........");
    }

    /* create first a new input file. */
    if ( rank == root ) {
	int fh, old_mask, perm;
	int veriarr[72];


	old_mask = umask(022);
	umask (old_mask);
	perm = old_mask^0666;

	fh = open ("readfile2.out", O_CREAT|O_WRONLY, perm );
	if ( -1 == fh ) {
	    printf("Could not create input file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    veriarr[0] = -1;
	    veriarr[1] = -2;
	    SL_write ( fh, veriarr, 2*sizeof(int));
	    
	    for ( i=0; i<72; i++ ) {
		veriarr[i] = i;
	    }
	    SL_write ( fh, veriarr, 72 * sizeof(int));
	    close (fh);
	}


    }


    memset ( inarr, 0, 12*sizeof(int));
    MPI_File_open ( comm, "readfile2.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );

    MPI_File_set_view ( file1, 8, MPI_INT, fview1, "native", MPI_INFO_NULL );

    MPI_File_read_at_all ( file1, 0, inarr, 6, MPI_INT, &status );
    MPI_File_read_at_all ( file1, 6, &inarr[6], 6, MPI_INT, &status );
    MPI_File_close ( &file1 );

    localret = 1;
    if ( ret == MPI_SUCCESS && ret2 == MPI_SUCCESS ) {
	localret = 0;
    }
    for ( i=0; i<6; i++ ) {
	if ( inarr[i] != rank*6+i ) 
	    localret = 1;
	if ( inarr[6+i] != rank*6 + 36 +i ) 
	    localret = 1;
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root ) {
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
	
	unlink("readfile2.out");
    }


/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
    if ( rank == root ) {
	printf("Checking for MPI_File_read_all_begin.......");
    }

    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );
    
    ret = MPI_File_read_all_begin ( file1, inarr, 12, MPI_INT );
    ret2 = MPI_File_read_all_end ( file1, inarr, &status );
    MPI_File_close ( &file1);
    
    localret = 0;
    if ( ret != MPI_SUCCESS || ret2 != MPI_SUCCESS ) {
	localret = 1;
    }
    /* verifying input data */
    for ( i=0; i<6; i++ ) {
	if ( inarr[i] != rank*6+i ) 
	    localret = 1;
	if ( inarr[6+i] != rank*6 + 36 +i ) 
	    localret = 1;
    }
    

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root ) {
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
	printf("Checking for MPI_File_read_at_all_begin....");
    }
    
    memset ( inarr, 0, 12*sizeof(int));
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );

    ret = MPI_File_read_at_all_begin ( file1, displs[0], inarr, 6, MPI_INT);
    ret2 = MPI_File_read_at_all_end ( file1, inarr, &status );
    ret3 = MPI_File_read_at_all_begin ( file1, displs[1], &inarr[6], 6, MPI_INT );
    ret4 = MPI_File_read_at_all_end ( file1, &inarr[6], &status );
    MPI_File_close ( &file1 );

    localret = 1;
    if ( ret == MPI_SUCCESS && ret2 == MPI_SUCCESS && ret3 == MPI_SUCCESS && ret4 == MPI_SUCCESS ) {
	localret = 0;
    }
    for ( i=0; i<6; i++ ) {
	if ( inarr[i] != rank*6+i ) 
	    localret = 1;
	if ( inarr[6+i] != rank*6 + 36 +i ) 
	    localret = 1;
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root ) {
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}
    }
/*******************************************************************************/	
/*******************************************************************************/	
/*******************************************************************************/	
    if ( rank == root ) {
	printf("Checking for MPI_File_iread_all............");
    }

    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );
    
    ret = MPI_File_iread_all ( file1, inarr, 12, MPI_INT, &req[0] );
    ret2 = MPI_Wait ( &req[0], &status );
    MPI_File_close ( &file1);
    
    localret = 0;
    if ( ret != MPI_SUCCESS || ret2 != MPI_SUCCESS ) {
	localret = 1;
    }
    /* verifying input data */
    for ( i=0; i<6; i++ ) {
	if ( inarr[i] != rank*6+i ) 
	    localret = 1;
	if ( inarr[6+i] != rank*6 + 36 +i ) 
	    localret = 1;
    }
    

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root ) {
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
	printf("Checking for MPI_File_iread_at_all.........");
    }
    
    memset ( inarr, 0, 12*sizeof(int));
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );

    ret = MPI_File_iread_at_all ( file1, displs[0], inarr, 6, MPI_INT, &req[0]);
    ret2 = MPI_File_iread_at_all ( file1, displs[1], &inarr[6], 6, MPI_INT, &req[1] );
    ret3 = MPI_Waitall ( 2, req, stats );
    MPI_File_close ( &file1 );

    localret = 1;
    if ( ret == MPI_SUCCESS && ret2 == MPI_SUCCESS && ret3 == MPI_SUCCESS ) {
	localret = 0;
    }
    for ( i=0; i<6; i++ ) {
	if ( inarr[i] != rank*6+i ) 
	    localret = 1;
	if ( inarr[6+i] != rank*6 + 36 +i ) 
	    localret = 1;
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root ) {
	if ( globalret == 0 ) {
	    printf("working\n");
	}
	else {
	    printf("false\n");
	    total++;
	}

	unlink("readfile1.out");
    }

    MPI_Type_free ( &fview1 );
    MPI_Type_free ( &fview2 );
    MPI_Type_free ( &tmp1 );
    MPI_Type_free ( &tmp2 );

#ifndef GLOBAL
    MPI_Finalize ();
#endif

    return total;
}