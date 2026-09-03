/*
** This file test MPI_File_set_view
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include "mpi.h"
#include "common.h"

#define BLOCK_SIZE 1000


/* Some macros for the Type_create_subarray test */
#define NG  1
#define NDIMS  4
#define NA  (size_t)256
#define NB  (size_t)128
#define NC  (size_t)128
#define ND  (size_t)32

#ifdef GLOBAL
int fileview_test ( MPI_Comm comm, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
#endif
    
    int rank, size, i, j, ret, localret, globalret;
    MPI_Status status;
    MPI_File file1;
    int readarr[12], inarr[12];
    MPI_Datatype fview, fview2, fview3, dats[2];
    MPI_Datatype tmp2, tmp3;
    int blength[2], displ;
    MPI_Aint displs[2];
    char *data=NULL;
    
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
    
    if ( rank == root ) {
	/* generate an input file */
	int fh, writearr[36];
	int old_mask, perm;
	
	for ( i=0; i<36; i++ ) {
	    writearr[i] = i;
	}
	
	
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
	    ssize_t r;
	    r = write ( fh, writearr, 36 * sizeof(int));
	    if ( r != 36 * sizeof(int)) {
		printf("Could not write input file\n");
		MPI_Abort ( comm, 1);
	    }
	    close (fh);
	}
	
	printf("Checking for MPI_File_set_view:\n");
	printf("    setting view using indexed type........");
    }
    
    MPI_Barrier ( comm );
    blength[0] = 6; 
    displ = rank * 6;
    MPI_Type_indexed (1, blength, &displ, MPI_INT, &fview );
    MPI_Type_commit ( &fview);
    
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );
    
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview, "native", MPI_INFO_NULL );
    MPI_File_read ( file1, readarr, 6, MPI_INT, &status);
    MPI_File_close ( &file1);
    
    localret = 0;
    for (i = 0 ; i<6; i++ ) {
	if (readarr[i] != rank*6 + i ) {
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
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
	printf("    setting view using vector type.........");
    }
    
    MPI_Type_vector ( 1, 6, 36, MPI_INT, &fview2);
    MPI_Type_commit ( &fview2);
    
    MPI_File_open ( comm, "readfile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );
    
    ret = MPI_File_set_view ( file1, rank*6*sizeof(int), MPI_INT, fview2, "native", MPI_INFO_NULL );
    MPI_File_read ( file1, readarr, 6, MPI_INT, &status);
    MPI_File_close ( &file1);
    MPI_Type_free ( &fview2);
    
    localret = 0;
    for (i = 0 ; i<6; i++ ) {
	if (readarr[i] != rank*6 + i ) {
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
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");

	/* clean up the files generated in this test case. */
	unlink ("readfile1.out");
    }
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
	// This next test case is based on an bug report on the mailing list 
	printf("    setting view using create_subarray.....");
        
        double *data;
        int sizes1[NDIMS] = {NA, NB, NC, ND};
        int subsizes1[NDIMS] ={NA, NB, NC, ND};
        int starts1[NDIMS] = {0, 0, 0, 0};
        
        int sizes2[NDIMS] = {NA + 2*NG, NB+2*NG, NC+2*NG, ND+2*NG};
        int subsizes2[NDIMS] ={NA, NB, NC, ND};
        int starts2[NDIMS] = {NG, NG, NG, NG};
        
        MPI_Datatype subtype, coretype;
        MPI_Offset off = 0;
        int fsize;
        long arrsize = (NA+2*NG)*(NB+2*NG)*(NC+2*NG)*(ND+2*NG);

        data = (double*) malloc( arrsize* sizeof(double));
        for ( i=0; i<arrsize; i++ ) {
            data[i] = (double) i;
        }
        
        MPI_Type_create_subarray(NDIMS, sizes1, subsizes1, starts1,
                                 MPI_ORDER_C, MPI_DOUBLE, &subtype);
        MPI_Type_commit(&subtype);
        MPI_Type_create_subarray(NDIMS, sizes2, subsizes2, starts2,
                                 MPI_ORDER_C, MPI_DOUBLE, &coretype);
        
        MPI_Type_commit(&coretype);
        MPI_Type_size(coretype, &size);
        
        unlink("writefile1.dat");
        MPI_File_open(MPI_COMM_SELF, "writefile1.dat", MPI_MODE_WRONLY +
                      MPI_MODE_CREATE, MPI_INFO_NULL, &file1);
        MPI_File_set_view(file1, off, MPI_DOUBLE, subtype, "native", MPI_INFO_NULL);
        
        ret = MPI_File_write(file1, data, 1, coretype, MPI_STATUS_IGNORE);
        MPI_File_get_size(file1, &off);
        if ( off == (MPI_Aint) size ) {
            printf("working\n");
        }
        else {
#ifdef DEBUG
            printf("Extents of subtype representing file on disk size %d actual size =%ld \n", size, off);
#endif
            printf("false\n");
        }
        MPI_File_close(&file1);
        
        MPI_Type_free(&coretype);
        MPI_Type_free(&subtype);
        free(data);
        unlink("writefile1.dat");

    }
    MPI_Barrier ( comm );
        	
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root )  { 
        
	printf("    resetting the file view................");
    }
    
    MPI_Barrier ( comm );
    
    for ( i=0; i<6; i++ ) {
	inarr[i] = rank*6+i;
    }

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_RDWR,
		    MPI_INFO_NULL, &file1 );
    
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview, "native", MPI_INFO_NULL );
    MPI_File_write ( file1, inarr, 6, MPI_INT, &status);

    ret = MPI_File_set_view ( file1, 0, MPI_BYTE, MPI_BYTE, "native", MPI_INFO_NULL );

    if ( rank == 0 ) {
        int veriarr[36];

        MPI_File_read ( file1, veriarr, 36, MPI_INT, &status);


        localret = 0;
        for (i = 0 ; i<3; i++ ) {
            if (veriarr[i] != rank*6 + i ) {
                localret = 1;
//#ifdef VERBOSE
                printf("%d: element %d is %d\n", rank, i, readarr[i] );
//#endif
            }
        }
    
        if ( ret != MPI_SUCCESS ) {
            localret = 1;
        }
    }
    else {
        localret = ret;
    }

    MPI_File_close ( &file1);
    MPI_Type_free ( &fview);

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
	printf("    using file view w/ displacement........");
    }
    
    blength[0] = 6;
    displs[0]  = rank*6*sizeof(int);
    dats[0]    = MPI_INT;
    MPI_Type_create_struct ( 1, blength, displs, dats, &tmp2);
    MPI_Type_commit ( &tmp2);
    MPI_Type_create_resized ( tmp2, 0, 36*sizeof(int), &fview2 );
    MPI_Type_commit ( &fview2);
    
    
    MPI_File_open ( comm, "writefile3.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );
    
    if ( rank == 0 ) {
	inarr[0] = -1;
	inarr[1] = -2;
	MPI_File_write ( file1, inarr, 2, MPI_INT, &status );
    }
    MPI_Barrier ( comm );
    ret = MPI_File_set_view ( file1, 8, MPI_INT, fview2, "native", MPI_INFO_NULL );
    
    for ( i=0; i<6; i++ ) {
	inarr[i] = rank*6+i;
	inarr[6+i] = rank*6 + 36 +i;
    }
    MPI_File_write ( file1, inarr, 12, MPI_INT, &status );
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
	    SL_read (fh, veriarr, 2*sizeof(int));
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
	}
	
	unlink("writefile3.out");
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
	printf("    using the file view repetitivly........");
    }
    
    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview2, "native", MPI_INFO_NULL );
    
    for ( i=0; i<6; i++ ) {
	inarr[i] = rank*6+i;
	inarr[6+i] = rank*6 + 36 +i;
    }
    MPI_File_write ( file1, inarr, 12, MPI_INT, &status );
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
	}
	
	
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
	unlink("writefile1.out");
	printf("    write less than the file view..........");
    }
    
    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview2, "native", MPI_INFO_NULL );
    
    MPI_File_write ( file1, &inarr[0], 3, MPI_INT, &status );
    MPI_File_write ( file1, &inarr[3], 3, MPI_INT, &status );
    MPI_File_write ( file1, &inarr[6], 3, MPI_INT, &status );
    MPI_File_write ( file1, &inarr[9], 3, MPI_INT, &status );
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
	}
	
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
	unlink("writefile1.out");
	printf("    writing a file view with gaps..........");
    }
    
    blength[0] = 6;
    blength[1] = 6;
    displs[0]  = rank*6*sizeof(int);
    displs[1]  = 36*sizeof(int)+rank*6*sizeof(int);
    dats[0]    = MPI_INT;
    dats[1]    = MPI_INT;
    MPI_Type_create_struct ( 2, blength, displs, dats, &tmp3);
    MPI_Type_commit ( &tmp3);
    MPI_Type_create_resized ( tmp3, 0, 72*sizeof(int), &fview3 );
    MPI_Type_commit ( &fview3);
    
    
    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    MPI_INFO_NULL, &file1 );
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview3, "native", MPI_INFO_NULL );
    
    MPI_File_write ( file1, inarr, 12, MPI_INT, &status );
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
	}
	
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
	printf("    reading a file view with gaps..........");
    }
    
    MPI_File_open ( comm, "writefile1.out", MPI_MODE_RDONLY,
		    MPI_INFO_NULL, &file1 );
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview3, "native", MPI_INFO_NULL );
    
    for ( i = 0; i< 12; i++ ) {
	inarr[i]=0;
    }
    MPI_File_read ( file1, inarr, 12, MPI_INT, &status );
    MPI_File_close ( &file1);
    
    localret=0;
    for (i=0; i<6; i++ ) {
	if ( inarr[i] != rank*6+i ) {
	    localret = 1;
#ifdef VERBOSE
	    printf("%d: Element %d is %d\n", rank, i, inarr[i]);
#endif
	}
	if ( inarr[6+i] != rank*6 + 36 +i ) {
	    localret = 1;
#ifdef VERBOSE
	    printf("%d: Element %d is %d\n", rank, (6+i), inarr[6+i]);
#endif
	}
    }
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	
	/* clean up the files generated in this test case. */
	unlink ("writefile1.out");
    }
    
    MPI_Type_free ( &fview2);
    MPI_Type_free ( &fview3);
    MPI_Type_free ( &tmp2);
    MPI_Type_free ( &tmp3);
    
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root )  { 
	// This next test case is based on an HDF5 test case 
	printf("    using file view of size zero...........");
    }
    
    displs[0] = 0;
    blength[0] = BLOCK_SIZE;
    
    data = (char *) malloc(BLOCK_SIZE);
    memset(data, 0, BLOCK_SIZE);
    
    if( root == rank) {
        MPI_Type_create_hindexed(1, blength, displs, MPI_BYTE, &fview2);
        MPI_Type_create_hvector(1, BLOCK_SIZE, 0, MPI_BYTE, &tmp2);
    }
    else {
        MPI_Type_create_hindexed(0, blength, displs, MPI_BYTE, &fview2);
        MPI_Type_create_hvector(0, BLOCK_SIZE, 0, MPI_BYTE, &tmp2);
    }
    
    MPI_Type_commit(&fview2);
    MPI_Type_commit(&tmp2);
    
    MPI_File_open (comm, "writefile1.out", MPI_MODE_RDWR | MPI_MODE_CREATE,
		   MPI_INFO_NULL, &file1); 

    MPI_File_set_view(file1, 2144, MPI_BYTE, 
		      fview2, "native", MPI_INFO_NULL);
    
    /* write everything */
    ret =  MPI_File_write_at_all (file1, 0, data, 
				  1, tmp2, MPI_STATUS_IGNORE);
    MPI_File_close(&file1);
    
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
    MPI_Type_free (&tmp2);
    MPI_Type_free(&fview2);
    free ( data );
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root )  { 
	printf("Checking for MPI_File_get_view.............");
    }
    MPI_File_open (comm, "writefile1.out", MPI_MODE_RDWR | MPI_MODE_CREATE, 
                   MPI_INFO_NULL, &file1 );
    
    MPI_Datatype etype, ftype;
    MPI_Offset disp;
    char datarep[64];
    ret = MPI_File_get_view (file1, &disp, &etype, &ftype, datarep);
    
    localret = 0;
    if ( ret != MPI_SUCCESS ) {
        localret = 1;
    }
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");

	printf("    verifying etype envelope...............");
    }
    
    int num_integers=-1, num_addresses=-1, num_datatypes=-1, combiner=-1;
    ret = MPI_Type_get_envelope ( etype, &num_integers, &num_addresses, 
                                  &num_datatypes, &combiner);
    localret = 0;
    if ( num_integers != 0 || num_addresses != 0 ||
         num_datatypes != 0 || combiner < 0  ||
         ret != MPI_SUCCESS )  {
        localret = 1;
#ifdef VERBOSE
        printf("etype: num_integers=%d, num_addresses=%d, num_datatypes=%d, "
               "combiner=%d\n", num_integers, num_addresses, num_datatypes, 
               combiner );
#endif        
    }
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
        if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
        
	printf("    verifying ftype envelope...............");
    }
    
    num_integers=-1, num_addresses=-1, num_datatypes=-1, combiner=-1;
    ret = MPI_Type_get_envelope ( ftype, &num_integers, &num_addresses, 
                                  &num_datatypes, &combiner);
    localret = 0;
    if ( num_integers != 0 || num_addresses != 0 ||
         num_datatypes != 0 || combiner < 0  ||
         ret != MPI_SUCCESS )  {
        localret = 1;
#ifdef VERBOSE
        printf("ftype: num_integers=%d, num_addresses=%d, num_datatypes=%d, "
               "combiner=%d\n", num_integers, num_addresses, num_datatypes, 
               combiner );
#endif        
    }
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
    }
    MPI_File_close ( &file1 );

    if ( rank == root ) {
        /* clean up the files generated in this test case. */
        unlink ("writefile1.out");
    }
    
#ifndef GLOBAL
    MPI_Finalize ();
#endif
    
    return 0;
}
