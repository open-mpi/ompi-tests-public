/*
** This file test a number of utility routines.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "mpi.h"
#include "common.h"

#define BLOCK_SIZE 1000

static int conv_isbigendian ( void );
static void conv_from_external32 ( int *a, int len );
static void conv_to_external32 ( int *a, int len );

#ifdef GLOBAL
int utils_test ( MPI_Comm comm, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
#endif
    
    int rank, size, i, j, ret, ret2, localret, globalret;
    MPI_Status status;
    MPI_File file1, file2;
    int readarr[12], inarr[12];
    MPI_Datatype tmp, fview, dats[2];
    int blength[2], displ;
    MPI_Aint displs[2];
    MPI_Offset fsize, offset;
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
    
    
    blength[0] = 6;
    blength[1] = 6;
    displs[0]  = rank*6*sizeof(int);
    displs[1]  = 36*sizeof(int)+rank*6*sizeof(int);
    dats[0]    = MPI_INT;
    dats[1]    = MPI_INT;
    MPI_Type_create_struct ( 2, blength, displs, dats, &tmp);
    MPI_Type_commit ( &tmp);
    MPI_Type_create_resized ( tmp, 0, 72*sizeof(int), &fview );
    MPI_Type_commit ( &fview );
    
    if ( rank == root )  { 
	// This next test case is based on a bug report by Lisandro Dalcin
	printf("Checking for MPI_File_get_byte_offset:\n");
	printf("   using default filew view................");
    }

    MPI_File_open (comm, "writefile1.out", MPI_MODE_RDWR | MPI_MODE_CREATE, 
                   MPI_INFO_NULL, &file1 );
    
    localret = 0;
    if ( rank == 0 ) {
    for ( i = 0; i < 4; ++i) {
        MPI_Offset offset;
        MPI_File_get_byte_offset(file1, i, &offset);
        if (offset != i) {
#ifdef VERBOSE
            printf("[%d]: byte offset for %d should be %d, but is %d\n",
                   rank, i, i, offset);
#endif
            localret = 1;
        }
    }
    }
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
    }

    if ( rank == root )  { 
	printf("   using filew view w/ gaps................");
    }
    
    localret=0;
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview, "native", MPI_INFO_NULL );
    for (i = 0; i < 6; ++i) {
        MPI_Offset offset, offset2,offset3;
        MPI_File_get_byte_offset(file1, i, &offset);
        MPI_File_get_byte_offset(file1, 6+i, &offset2);
        MPI_File_get_byte_offset(file1, 12+i, &offset3);
        if ( offset != ((rank*6)+i)*sizeof(int) ) { 
#ifdef VERBOSE
            printf("[%d]: byte offset for %d should be %d, but is %d\n",
                   rank, i, (((rank*6)+i)*sizeof(int)), offset);
#endif
            localret = 1;
        }

        if ( offset2 != ((rank*6)+i+36)*sizeof(int)  ) {
#ifdef VERBOSE
            printf("[%d]: byte offset2 for %d should be %d, but is %d\n",
                   rank, i, (((rank*6)+i+36)*sizeof(int)), offset2);
#endif
            localret = 1;
        }

        if ( offset3 != ((rank*6)+i+72)*sizeof(int) ) {
#ifdef VERBOSE
            printf("[%d]: byte offset2 for %d should be %d, but is %d\n",
                   rank, i, (((rank*6)+i+72)*sizeof(int)), offset3);
#endif
            localret = 1;
        }

    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}

        /* clean up the files generated in this test case. */
        unlink ("writefile1.out");

    }
    MPI_File_close ( &file1 );
    MPI_Type_free ( &tmp);
    MPI_Type_free ( &fview);

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root )  { 
	// This next test case is based on a bug report by Lisandro Dalcin
	printf("Checking for MPI_File_preallocate:\n");
	printf("   using larger file size..................");


        MPI_File_open (MPI_COMM_SELF, "writefile1.out", MPI_MODE_RDWR | MPI_MODE_CREATE, 
                       MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_preallocate ( file1, 100 ) ;
        localret = MPI_File_get_size ( file1, &fsize );

        if ( 100 != fsize || ret != MPI_SUCCESS || localret != MPI_SUCCESS ) {
            printf("false\n");
            total++;
        }
        else {
            printf("working\n");
        }

	printf("   using less then current file size.......");        

        ret = MPI_File_preallocate ( file1, 10 ) ;
        localret = MPI_File_get_size ( file1, &fsize );

        if ( 100 != fsize || ret != MPI_SUCCESS || localret != MPI_SUCCESS ) {
            printf("false\n");
            total++;
        }
        else {
            printf("working\n");
        }
        MPI_File_close (&file1);
        unlink ("writefile1.out");
        
    }
    MPI_Barrier ( comm );
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if (rank == root)  {
	// This next test case is based on a bug report by @tukss on github
	int fh, old_mask, perm;
	int *tarr = calloc (2097152, sizeof(int));
	if (NULL == tarr) {
	    printf("Could not allocate memory\n");
	    MPI_Abort (comm , -1);
	}

	/* Create a small and a large input file first */
	/* generate a file first. */
	old_mask = umask(022);
	umask (old_mask);
	perm = old_mask^0666;

	fh = open ("smallfile.txt", O_CREAT|O_WRONLY, perm );
	if (-1 == fh) {
	    printf("Could not create small input file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    SL_write (fh, tarr, 4096);
	    close (fh);
	}

	fh = open ("largefile.txt", O_CREAT|O_WRONLY, perm );
	if ( -1 == fh ) {
	    printf("Could not create large input file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
	    SL_write (fh, tarr, 8388608);
	    close (fh);
	}

        MPI_File_open (MPI_COMM_SELF, "smallfile.txt", MPI_MODE_RDONLY,
                       MPI_INFO_NULL, &file1);
        MPI_File_open (MPI_COMM_SELF, "largefile.txt", MPI_MODE_RDONLY,
                       MPI_INFO_NULL, &file2);
	MPI_Offset small_file_size, large_file_size;
	MPI_File_get_size (file1, &small_file_size);
	MPI_File_get_size (file2, &large_file_size);

	printf("Checking for MPI_File_seek:\n");
	printf("   using small file with SEEK_SET..........");

	int pos = -4;
	ret = MPI_File_seek (file1, small_file_size + pos, MPI_SEEK_SET);
	ret2 = MPI_File_get_position (file1, &offset);

	if ((MPI_SUCCESS == ret) && (MPI_SUCCESS == ret2) && (4092 == offset)) {
	    printf("working\n");
	} else {
	    printf("false\n");
	    total++;
	}

	printf("   using small file with SEEK_END..........");
	offset = 0;
	ret = MPI_File_seek (file1, pos, MPI_SEEK_END);
	ret2 = MPI_File_get_position (file1, &offset);

	if ((MPI_SUCCESS == ret) && (MPI_SUCCESS == ret2) && (4092 == offset)) {
	    printf("working\n");
	} else {
	    printf("false\n");
	    total++;
	}

	printf("   using larger file with SEEK_SET.........");
	offset = 0;
	ret = MPI_File_seek (file2, large_file_size + pos, MPI_SEEK_SET);
	ret2 = MPI_File_get_position (file2, &offset);

	if ((MPI_SUCCESS == ret) && (MPI_SUCCESS == ret2) && (8388604 == offset)) {
	    printf("working\n");
	} else {
	    printf("false\n");
	    total++;
	}
	printf("   using larger file with SEEK_END.........");
	offset = 0;
	ret = MPI_File_seek (file2, pos, MPI_SEEK_END);
	ret2 = MPI_File_get_position (file2, &offset);

	if ((MPI_SUCCESS == ret) && (MPI_SUCCESS == ret2) && (8388604 == offset)) {
	    printf("working\n");
	} else {
	    printf("false\n");
	    total++;
	}

	unlink ("smallfile.txt");
	unlink ("largefile.txt");
    }
    MPI_Barrier (comm);
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    for ( i=0; i<6; i++ ) {
	inarr[i] = rank*6+i;
	inarr[6+i] = rank*6 + 36 +i;
    }

    if ( rank == root ) {
	printf("Checking for external32 data representation:\n");
	printf("   using MPI_File_write....................");
        
        ret = MPI_File_open ( MPI_COMM_SELF, "writefile.out", MPI_MODE_WRONLY | MPI_MODE_CREATE,
                              MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "external32", MPI_INFO_NULL );
        if ( ret != MPI_SUCCESS ) {
            char errstring[64];
            int s=64;
            MPI_Error_string( ret, errstring, &s);
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
        }
        MPI_File_write ( file1, inarr, 6, MPI_INT, &status);
        MPI_File_close ( &file1);
        
        int fh, readarr[6];
        
        /* Verify content of the file using posix read operations */
        fh = open ("writefile.out", O_RDONLY );
        if ( -1 == fh ) {
            localret = 1;
        }
        else {
            SL_read ( fh, readarr, 6 * sizeof(int));
            conv_from_external32 ( readarr, 6);
            localret = 0;
	    for ( i = 0; i< 6; i++ ) {
		if ( readarr[i] != rank*6+ i ) {
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
        
        /* clean up the files generated in this test case. */
        unlink ( "writefile.out");
    }
    MPI_Barrier ( comm );
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
	printf("   using MPI_File_write_at.................");
        
        ret = MPI_File_open ( MPI_COMM_SELF, "writefile.out", MPI_MODE_WRONLY | MPI_MODE_CREATE,
                              MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "external32", MPI_INFO_NULL );
        if ( ret != MPI_SUCCESS ) {
            char errstring[64];
            int s=64;
            MPI_Error_string( ret, errstring, &s);
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
        }
        MPI_File_write_at ( file1, 0, inarr, 6, MPI_INT, &status);
        MPI_File_close ( &file1);
        
        int fh, readarr[6];
        
        /* Verify content of the file using posix read operations */
        fh = open ("writefile.out", O_RDONLY );
        if ( -1 == fh ) {
            localret = 1;
        }
        else {
            SL_read ( fh, readarr, 6 * sizeof(int));
            conv_from_external32 ( readarr, 6);
            localret = 0;
	    for ( i = 0; i< 6; i++ ) {
		if ( readarr[i] != rank*6+ i ) {
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
        
        /* clean up the files generated in this test case. */
        unlink ( "writefile.out");
    }
    MPI_Barrier ( comm );
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
        MPI_Request req;
	printf("   using MPI_File_iwrite...................");
        
        ret = MPI_File_open ( MPI_COMM_SELF, "writefile.out", MPI_MODE_WRONLY | MPI_MODE_CREATE,
                              MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "external32", MPI_INFO_NULL );
        if ( ret != MPI_SUCCESS ) {
            char errstring[64];
            int s=64;
            MPI_Error_string( ret, errstring, &s);
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
        }
        MPI_File_iwrite ( file1, inarr, 6, MPI_INT, &req);
        MPI_Wait ( &req, &status );
        MPI_File_close ( &file1);
        
        int fh, readarr[6];
        
        /* Verify content of the file using posix read operations */
        fh = open ("writefile.out", O_RDONLY );
        if ( -1 == fh ) {
            localret = 1;
        }
        else {
            SL_read ( fh, readarr, 6 * sizeof(int));
            conv_from_external32 ( readarr, 6);
            localret = 0;
	    for ( i = 0; i< 6; i++ ) {
		if ( readarr[i] != rank*6+ i ) {
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
        
        /* clean up the files generated in this test case. */
        unlink ( "writefile.out");
    }
    MPI_Barrier ( comm );
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
        MPI_Request req;
	printf("   using MPI_File_iwrite_at................");
        
        ret = MPI_File_open ( MPI_COMM_SELF, "writefile.out", MPI_MODE_WRONLY | MPI_MODE_CREATE,
                              MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "external32", MPI_INFO_NULL );
        if ( ret != MPI_SUCCESS ) {
            char errstring[64];
            int s=64;
            MPI_Error_string( ret, errstring, &s);
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
        }
        MPI_File_iwrite_at ( file1, 0, inarr, 6, MPI_INT, &req);
        MPI_Wait ( &req, &status );
        MPI_File_close ( &file1);
        
        int fh, readarr[6];
        
        /* Verify content of the file using posix read operations */
        fh = open ("writefile.out", O_RDONLY );
        if ( -1 == fh ) {
            localret = 1;
        }
        else {
            SL_read ( fh, readarr, 6 * sizeof(int));
            conv_from_external32 ( readarr, 6);
            localret = 0;
	    for ( i = 0; i< 6; i++ ) {
		if ( readarr[i] != rank*6+ i ) {
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
        
        /* clean up the files generated in this test case. */
        unlink ( "writefile.out");
    }
    MPI_Barrier ( comm );
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
	printf("   using MPI_File_write_shared.............");
        
        ret = MPI_File_open ( MPI_COMM_SELF, "writefile.out", MPI_MODE_WRONLY | MPI_MODE_CREATE,
                              MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "external32", MPI_INFO_NULL );
        if ( ret != MPI_SUCCESS ) {
            char errstring[64];
            int s=64;
            MPI_Error_string( ret, errstring, &s);
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
        }
        MPI_File_write_shared ( file1, inarr, 6, MPI_INT, &status);
        MPI_File_close ( &file1);
        
        int fh, readarr[6];
        
        /* Verify content of the file using posix read operations */
        fh = open ("writefile.out", O_RDONLY );
        if ( -1 == fh ) {
            localret = 1;
        }
        else {
            SL_read ( fh, readarr, 6 * sizeof(int));
            conv_from_external32 ( readarr, 6);
            localret = 0;
	    for ( i = 0; i< 6; i++ ) {
		if ( readarr[i] != rank*6+ i ) {
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
        
        /* clean up the files generated in this test case. */
        unlink ( "writefile.out");
    }
    MPI_Barrier ( comm );
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
        MPI_Request req;
	printf("   using MPI_File_iwrite_shared............");
        
        ret = MPI_File_open ( MPI_COMM_SELF, "writefile.out", MPI_MODE_WRONLY | MPI_MODE_CREATE,
                              MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "external32", MPI_INFO_NULL );
        if ( ret != MPI_SUCCESS ) {
            char errstring[64];
            int s=64;
            MPI_Error_string( ret, errstring, &s);
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
        }
        MPI_File_iwrite_shared ( file1, inarr, 6, MPI_INT, &req);
        MPI_Wait ( &req, &status );
        MPI_File_close ( &file1);
        
        int fh, readarr[6];
        
        /* Verify content of the file using posix read operations */
        fh = open ("writefile.out", O_RDONLY );
        if ( -1 == fh ) {
            localret = 1;
        }
        else {
            SL_read ( fh, readarr, 6 * sizeof(int));
            conv_from_external32 ( readarr, 6);
            localret = 0;
	    for ( i = 0; i< 6; i++ ) {
		if ( readarr[i] != rank*6+ i ) {
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
        
        /* clean up the files generated in this test case. */
        unlink ( "writefile.out");
    }
    MPI_Barrier ( comm );
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
	printf("   using MPI_File_read.....................");

	int fh, writearr[6];
	int old_mask, perm;

	for ( i=0; i<6; i++ ) {
	    writearr[i] = i;
	}

	/* generate a file first. */
	old_mask = umask(022);
	umask (old_mask);
	perm = old_mask^0666;

	fh = open ("readfile.out", O_CREAT|O_WRONLY, perm );
	if ( -1 == fh ) {
	    printf("Could not create input file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
            conv_to_external32 ( writearr, 6 );
	    SL_write ( fh, writearr, 6 * sizeof(int));
	    close (fh);
	}
        
        ret = MPI_File_open ( MPI_COMM_SELF, "readfile.out", MPI_MODE_RDONLY,
                              MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "external32", MPI_INFO_NULL );
        if ( ret != MPI_SUCCESS ) {
            char errstring[64];
            int s=64;
            MPI_Error_string( ret, errstring, &s);
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
        }
        MPI_File_read ( file1, readarr, 6, MPI_INT, &status);
        MPI_File_close ( &file1);
        localret = 0;
        for ( i = 0; i< 6; i++ ) {
            if ( readarr[i] != rank*6+ i ) {
#ifdef VERBOSE
                printf("Element %d is %d\n", i, readarr[i] );
#endif
                localret = 1;
            }
        }
	if ( ret == MPI_SUCCESS && localret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
        
    }    
    MPI_Barrier ( comm );

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
	printf("   using MPI_File_read_at..................");

	int writearr[6];
	for ( i=0; i<6; i++ ) {
	    writearr[i] = i;
	}
        ret = MPI_File_open ( MPI_COMM_SELF, "readfile.out", MPI_MODE_RDONLY,
                              MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "external32", MPI_INFO_NULL );
        if ( ret != MPI_SUCCESS ) {
            char errstring[64];
            int s=64;
            MPI_Error_string( ret, errstring, &s);
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
        }
        MPI_File_read_at ( file1, 0, readarr, 6, MPI_INT, &status);
        MPI_File_close ( &file1);
        localret = 0;
        for ( i = 0; i< 6; i++ ) {
            if ( readarr[i] != rank*6+ i ) {
#ifdef VERBOSE
                printf("Element %d is %d\n", i, readarr[i] );
#endif
                localret = 1;
            }
        }
	if ( ret == MPI_SUCCESS && localret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
        
    }    
    MPI_Barrier ( comm );

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
        MPI_Request req;
	printf("   using MPI_File_iread....................");

	int writearr[6];
	for ( i=0; i<6; i++ ) {
	    writearr[i] = i;
	}
        ret = MPI_File_open ( MPI_COMM_SELF, "readfile.out", MPI_MODE_RDONLY,
                              MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "external32", MPI_INFO_NULL );
        if ( ret != MPI_SUCCESS ) {
            char errstring[64];
            int s=64;
            MPI_Error_string( ret, errstring, &s);
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
        }
        MPI_File_iread ( file1, readarr, 6, MPI_INT, &req);
        MPI_Wait ( &req, &status );
        MPI_File_close ( &file1);
        localret = 0;
        for ( i = 0; i< 6; i++ ) {
            if ( readarr[i] != rank*6+ i ) {
#ifdef VERBOSE
                printf("Element %d is %d\n", i, readarr[i] );
#endif
                localret = 1;
            }
        }
	if ( ret == MPI_SUCCESS && localret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
        
    }    
    MPI_Barrier ( comm );

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
        MPI_Request req;
	printf("   using MPI_File_iread_at.................");

	int writearr[6];
	for ( i=0; i<6; i++ ) {
	    writearr[i] = i;
	}
        ret = MPI_File_open ( MPI_COMM_SELF, "readfile.out", MPI_MODE_RDONLY,
                              MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "external32", MPI_INFO_NULL );
        if ( ret != MPI_SUCCESS ) {
            char errstring[64];
            int s=64;
            MPI_Error_string( ret, errstring, &s);
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
        }
        MPI_File_iread_at ( file1, 0, readarr, 6, MPI_INT, &req);
        MPI_Wait ( &req, &status );
        MPI_File_close ( &file1);
        localret = 0;
        for ( i = 0; i< 6; i++ ) {
            if ( readarr[i] != rank*6+ i ) {
#ifdef VERBOSE
                printf("Element %d is %d\n", i, readarr[i] );
#endif
                localret = 1;
            }
        }
	if ( ret == MPI_SUCCESS && localret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
        
    }    
    MPI_Barrier ( comm );

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
	printf("   using MPI_File_read_shared..............");

	int writearr[6];
	for ( i=0; i<6; i++ ) {
	    writearr[i] = i;
	}
        ret = MPI_File_open ( MPI_COMM_SELF, "readfile.out", MPI_MODE_RDONLY,
                              MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "external32", MPI_INFO_NULL );
        if ( ret != MPI_SUCCESS ) {
            char errstring[64];
            int s=64;
            MPI_Error_string( ret, errstring, &s);
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
        }
        MPI_File_read_shared ( file1, readarr, 6, MPI_INT, &status);
        MPI_File_close ( &file1);
        localret = 0;
        for ( i = 0; i< 6; i++ ) {
            if ( readarr[i] != rank*6+ i ) {
#ifdef VERBOSE
                printf("Element %d is %d\n", i, readarr[i] );
#endif
                localret = 1;
            }
        }
	if ( ret == MPI_SUCCESS && localret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
        
    }    
    MPI_Barrier ( comm );

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
        MPI_Request req;
	printf("   using MPI_File_iread_shared.............");

	int writearr[6];
	for ( i=0; i<6; i++ ) {
	    writearr[i] = i;
	}
        ret = MPI_File_open ( MPI_COMM_SELF, "readfile.out", MPI_MODE_RDONLY,
                              MPI_INFO_NULL, &file1 );
        
        ret = MPI_File_set_view ( file1, 0, MPI_INT, MPI_INT, "external32", MPI_INFO_NULL );
        if ( ret != MPI_SUCCESS ) {
            char errstring[64];
            int s=64;
            MPI_Error_string( ret, errstring, &s);
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
        }
        MPI_File_iread_shared ( file1, readarr, 6, MPI_INT, &req);
        MPI_Wait ( &req, &status );
        MPI_File_close ( &file1);
        localret = 0;
        for ( i = 0; i< 6; i++ ) {
            if ( readarr[i] != rank*6+ i ) {
#ifdef VERBOSE
                printf("Element %d is %d\n", i, readarr[i] );
#endif
                localret = 1;
            }
        }
	if ( ret == MPI_SUCCESS && localret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
        
    }    
    MPI_Barrier ( comm );

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    MPI_Datatype fview1, tmp1;
    
    /* create first the derived data types required for the file views. */
    blength[0] = 6;
    displs[0]  = rank*6*sizeof(int);
    dats[0]    = MPI_INT;
    MPI_Type_create_struct (1, blength, displs, dats, &tmp1);
    MPI_Type_commit (&tmp1 );
    MPI_Type_create_resized (tmp1, 0, 36*sizeof(int), &fview1);
    MPI_Type_commit (&fview1);
    
    if ( rank == root ) {
	printf("   using MPI_File_write_all................");
    }        
    ret = MPI_File_open ( comm, "writefile.out", MPI_MODE_WRONLY | MPI_MODE_CREATE,
                          MPI_INFO_NULL, &file1 );
        
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview1, "external32", MPI_INFO_NULL );
    if ( ret != MPI_SUCCESS ) {
        char errstring[64];
        int s=64;
        MPI_Error_string( ret, errstring, &s);
        if ( rank == root ) 
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
    }
    MPI_File_write_all ( file1, inarr, 12, MPI_INT, &status);
    MPI_File_close ( &file1);

    localret = 0;    
    if ( rank == root ) {
        int fh, readarr[72];
        
        /* Verify content of the file using posix read operations */
        fh = open ("writefile.out", O_RDONLY );
        if ( -1 == fh ) {
            localret = 1;
        }
        else {
            SL_read ( fh, readarr, 72 * sizeof(int));
            conv_from_external32 ( readarr, 72);
            localret = 0;
	    for ( i = 0; i< 72; i++ ) {
		if ( readarr[i] != i ) {
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, readarr[i] );
#endif
		    localret = 1;
		}
	    }
	    close (fh);
	}
    }
    if ( ret != MPI_SUCCESS ) {
	localret = 1;
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
        
        /* clean up the files generated in this test case. */
        unlink ("writefile.out");
    }
    MPI_Barrier ( comm );
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
	printf("   using MPI_File_write_at_all.............");
    }        
    ret = MPI_File_open ( comm, "writefile.out", MPI_MODE_WRONLY | MPI_MODE_CREATE,
                          MPI_INFO_NULL, &file1 );
        
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview1, "external32", MPI_INFO_NULL );
    if ( ret != MPI_SUCCESS ) {
        char errstring[64];
        int s=64;
        MPI_Error_string( ret, errstring, &s);
        if ( rank == root ) 
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
    }
    MPI_File_write_at_all ( file1, 0, inarr, 12, MPI_INT, &status);
    MPI_File_close ( &file1);

    localret = 0;    
    if ( rank == root ) {
        int fh, readarr[72];
        
        /* Verify content of the file using posix read operations */
        fh = open ("writefile.out", O_RDONLY );
        if ( -1 == fh ) {
            localret = 1;
        }
        else {
            SL_read ( fh, readarr, 72 * sizeof(int));
            conv_from_external32 ( readarr, 72);
            localret = 0;
	    for ( i = 0; i< 72; i++ ) {
		if ( readarr[i] != i ) {
#ifdef VERBOSE
		    printf("Element %d is %d\n", i, readarr[i] );
#endif
		    localret = 1;
		}
	    }
	    close (fh);
	}
    }
    if ( ret != MPI_SUCCESS ) {
	localret = 1;
    }

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else {
	    printf("false\n");
	    total++;
	}
        
        /* clean up the files generated in this test case. */
        unlink ("writefile.out");
    }
    MPI_Barrier ( comm );
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
	printf("   using MPI_File_read_all.................");

	int fh, writearr[72];
	int old_mask, perm;

	for ( i=0; i<72; i++ ) {
	    writearr[i] = i;
	}

	/* generate a file first. */
	old_mask = umask(022);
	umask (old_mask);
	perm = old_mask^0666;

	fh = open ("readfile.out", O_CREAT|O_WRONLY, perm );
	if ( -1 == fh ) {
	    printf("Could not create input file \n");
	    MPI_Abort ( comm , -1 );
	}
	else {
            conv_to_external32 ( writearr, 72 );
	    SL_write ( fh, writearr, 72 * sizeof(int));
	    close (fh);
	}

    }        
    ret = MPI_File_open ( comm, "readfile.out", MPI_MODE_WRONLY | MPI_MODE_CREATE,
                          MPI_INFO_NULL, &file1 );
        
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview1, "external32", MPI_INFO_NULL );
    if ( ret != MPI_SUCCESS ) {
        char errstring[64];
        int s=64;
        MPI_Error_string( ret, errstring, &s);
        if ( rank == root ) 
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
    }
    MPI_File_read_all ( file1, inarr, 12, MPI_INT, &status);
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
        // unlink ("readfile.out");
    }

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    if ( rank == root ) {
	printf("   using MPI_File_read_at_all..............");
    }        
    ret = MPI_File_open ( comm, "readfile.out", MPI_MODE_WRONLY | MPI_MODE_CREATE,
                          MPI_INFO_NULL, &file1 );
        
    ret = MPI_File_set_view ( file1, 0, MPI_INT, fview1, "external32", MPI_INFO_NULL );
    if ( ret != MPI_SUCCESS ) {
        char errstring[64];
        int s=64;
        MPI_Error_string( ret, errstring, &s);
        if ( rank == root ) 
            printf("\n Could not set file view for external32 error code is %d %s\n",
                   ret, errstring);
    }
    MPI_File_read_at_all ( file1, 0, inarr, 12, MPI_INT, &status);
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
        unlink ("readfile.out");
    }
    
#ifndef GLOBAL
    MPI_Finalize ();
#endif

    return total;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

/* The next routines are based on source code from the FT-MPI library written approx. 2003.
Don't ask, don't tell :-) 
*/
static int conv_isbigendian ( void )
{
    const unsigned int value=0x12345678;
    const char *ptr = (char*)&value;
    int x=0;

    if ( sizeof(int) == 8 )
        x = 4;

    if ( ptr[x] == 0x12)
        return ( 1 ); /* big endian, true */
    else if ( ptr[x] == 0x78 )
        return ( 0 ); /* little endian, false */
    else
        printf ("Unkwon endia type ! What shall I ?\n");


    return ( -1 );
}

static void conv_from_external32 ( int *a, int len )
{
    int i, temp;
    
    if ( conv_isbigendian() ) {
        /* nothing to do */
        return;       
    }
    /* convert all elements from big endian to little endian */
    for ( i=0; i<len; i++ ) {
        temp = ntohl ( a[i]);
        a[i] = temp;
    }

    return;
}

static void conv_to_external32 ( int *a, int len )
{
    int i, temp;
    
    if ( conv_isbigendian() ) {
        /* nothing to do */
        return;       
    }
    /* convert all elements from big endian to little endian */
    for ( i=0; i<len; i++ ) {
        temp = htonl ( a[i]);
        a[i] = temp;
    }

    return;
}

