/*
** This various info objects 
** This is mostly relevant when looking into the MPI
** source code to see what is happening, but an easy
** way to get confirmation is to set the new
** mca_io_ompio_verbose_info_parsing parameter to 1 ( or 2)
** to see whether ompio correctly recognizes the values.
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

int write_all_2D ( MPI_Comm comm, int root );

#ifdef GLOBAL
int info_test ( MPI_Comm comm, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
#endif

    int rank, size, i, ret, ret2, ret3, ret4, localret, globalret, flag, isGpfs;
    MPI_Status status;
    MPI_File file1;
    int inarr[12], count, *inarr2=NULL, displs2[2];
    MPI_Datatype fview1, dats[2];
    MPI_Datatype tmp1;
    int blength[2];
    MPI_Aint displs[2];
    MPI_Info info;
    int print_line_break=0;

#ifndef GLOBAL
    MPI_Init ( &argc, &argv );
    if ( argc == 2 )  sleep ( 20 );
    if ( argc == 3 ) print_line_break=1;
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
    

    /* initialize input data */
    for ( i=0; i<6; i++ ) {
	inarr[i] = rank*6+i;
	inarr[6+i] = rank*6 + 36 +i;
    }

    if ( rank == root ) {
	printf("Testing File I/O with various Info objects:\n");
	printf("   using HINT collective_buffering.........");
        if ( print_line_break ) printf("\n");
    }
    MPI_Info_create( &info);
    MPI_Info_set(info, "collective_buffering", "true");

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    info, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );

    /* This operation should use the individual fcoll module */
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
	}
        unlink("writefile1.out");
    }
    MPI_Info_free ( &info);

/************************************************************************/
/************************************************************************/
/************************************************************************/
    if ( rank == root ) {
	printf("   using HINT cb_nodes.....................");
        if ( print_line_break ) printf("\n");
    }

    MPI_Info_create( &info);
    MPI_Info_set(info, "cb_nodes", "2");

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    info, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );

    /* This operation should use 2 aggregators in the vulcan, two_phase or dynamic 
    ** fcoll component 
    */
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
	}
        unlink("writefile1.out");
    }
    MPI_Info_free ( &info);


/************************************************************************/
/************************************************************************/
/************************************************************************/
    if ( rank == root ) {
	printf("   using HINT cb_buffer_size...............");
        if ( print_line_break ) printf("\n");
    }
    MPI_Info_create( &info);
    MPI_Info_set(info, "cb_buffer_size", "1048576");

    MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
		    info, &file1 );
    MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );


    /* 
    ** This operation should use 1MB temporary buffer in the vulcan, two_phase 
    ** or dynamic fcoll component 
    */
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
	}
        unlink("writefile1.out");
    }
    MPI_Info_free ( &info);

/************************************************************************/
/************************************************************************/
/************************************************************************/

    // TODO: check this
    isGpfsFS(".", &isGpfs);

    if(isGpfs) {

        if ( rank == root ) {
            printf("   setting All GPFS hints.....................");
            if ( print_line_break ) printf("\n");
        }

        MPI_Info_create( &info);

        // gpfsAccessRange Hint: start,length,isWrite :  length=0 indicates to end of file
        MPI_Info_set(info, "gpfsAccessRange", "0,0,1");
        MPI_Info_set(info, "gpfsFreeRange", "0,0");
        MPI_Info_set(info, "gpfsClearFileCache", "true");
        MPI_Info_set(info, "gpfsCancelHints", "true");
        // TODO: check if we need many tokens in value
        MPI_Info_set(info, "gpfsSetReplication", "2");
        // blocks (size st_blksize) to restripe set to 2M starting from 0
        // GPFS_FCNTL_RESTRIPE_RANGE_R
        // Probably cannot be done unless the file already exist: gpfs_fcntl.c says: The data movement is always done immediately.
        // MPI_Info_set(info, "gpfsByteRange", "0,33554432");
        // Rebalance file data: Options are defined under gpfs_fcntl.h, e.g: GPFS_FCNTL_RESTRIPE_B
        MPI_Info_set(info, "gpfsRestripeData", "10");

        MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
                        info, &file1 );
        MPI_File_close ( &file1);

        if ( rank == 0 ) {
            printf("working (If no error printed out.)\n");
            unlink("writefile1.out");
        }
        MPI_Info_free ( &info);

        /************************************************************************/
        if ( rank == root ) {
            printf("   using GPFS HINT gpfsAccessRange.....................");
            if ( print_line_break ) printf("\n");
        }

        MPI_Info_create( &info);
        // gpfsAccessRange Hint: start,length,isWrite :  length=0 indicates to end of file
        MPI_Info_set(info, "gpfsAccessRange", "0,0,1");
        MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
                        info, &file1 );
        MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );

        ret = MPI_File_write_all ( file1, inarr, 12, MPI_INT, &status );
        MPI_File_close ( &file1);

        localret = (MPI_SUCCESS == ret) ? 0 : 1;
        MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

        if ( rank == root ) {
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
            }
            unlink("writefile1.out");
        }
        MPI_Info_free ( &info);

        /************************************************************************/
        if ( rank == root ) {
            printf("   using GPFS HINT gpfsClearFileCache.....................");
            if ( print_line_break ) printf("\n");
        }

        MPI_Info_create( &info);
        MPI_Info_set(info, "gpfsClearFileCache", "true");
        MPI_File_open ( comm, "writefile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY,
                        info, &file1 );
        MPI_File_set_view ( file1, 0, MPI_INT, fview1, "native", MPI_INFO_NULL );
        ret = MPI_File_write_all ( file1, inarr, 12, MPI_INT, &status );
        MPI_File_close ( &file1);

        localret = (MPI_SUCCESS == ret) ? 0 : 1;
        MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

        if ( rank == root ) {
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
            }
            unlink("writefile1.out");
        }
        MPI_Info_free ( &info);
    }

    /************************************************************************/

    MPI_Type_free ( &fview1 );
    MPI_Type_free ( &tmp1 );

#ifndef GLOBAL
    MPI_Finalize ();
#endif

    return 0;
}
