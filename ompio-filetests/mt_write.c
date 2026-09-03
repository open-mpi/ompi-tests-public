/*
** This file test MPI_File_write
*/
#define _GNU_SOURCE 
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "mpi.h"
#include "common.h"

static void *write_test( void* arg);
static void *write_at_test( void* arg);

MPI_File *files=NULL;
MPI_Comm *comms=NULL;
pthread_t *tids = NULL;
int rank, size;
pthread_barrier_t   barrier; // barrier synchronization object


static void * write_test ( void *arg )
{
    int ret= MPI_SUCCESS;
    long myret = NULL;
    pthread_t t_id = pthread_self();
    int i, max_num_threads = (int) ((long)arg);
    int writearr[6];

    for ( i=0; i<6; i++ ) {
        writearr[i] = rank * 6 + i;
    }

    pthread_barrier_wait( &barrier );
    for (i=0; i< max_num_threads; i++ ) {
        if ( tids[i] == t_id ) break;
    }

#ifdef VERBOSE
    printf("%d / %d: before File_write\n", rank, i);
#endif
    ret = MPI_File_write ( files[i], writearr, 6, MPI_INT, MPI_STATUS_IGNORE);
    if ( MPI_SUCCESS != ret ) {
#ifdef VERBOSE
        printf("%d / %d error in File_write, error code %d\n", rank, i, ret);
#endif
        myret = 1;
    }
#ifdef VERBOSE    
    printf("%d / %d: after File_write \n", rank, i);
#endif
    pthread_exit ((void *) myret);
    return NULL;
}


static void * write_at_test ( void *arg )
{
    int ret= MPI_SUCCESS;
    long myret = NULL;
    pthread_t t_id = pthread_self();
    int i, ii, max_num_threads = (int) ((long)arg);
    int writearr[6];

    
    pthread_barrier_wait( &barrier );
    for (i=0; i< max_num_threads; i++ ) {
        if ( tids[i] == t_id ) break;
    }

    int realrank = rank * max_num_threads + i;
    MPI_Aint  offset = realrank * 6 * sizeof(int); 
    for ( ii=0; ii<6; ii++ ) {
        writearr[ii] = realrank * 6 + ii;
    }


#ifdef VERBOSE
    printf("%d / %d: before File_write_at\n", rank, i);
#endif
    ret = MPI_File_write_at ( files[0], offset, writearr, 6, MPI_INT, MPI_STATUS_IGNORE);
    if ( MPI_SUCCESS != ret ) {
#ifdef VERBOSE
        printf("%d / %d error in File_write_at, error code %d\n", rank, i, ret);
#endif
        myret = 1;
    }
#ifdef VERBOSE    
    printf("%d / %d: after File_write_at \n", rank, i);
#endif
    pthread_exit ((void *) myret);
    return NULL;
}


#ifdef GLOBAL
int mt_write_test ( MPI_Comm comm, int num_threads, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int i, root=0;
    int num_threads=3;
#endif
    MPI_Datatype fview;
    int blength[2], displ;

    int ret, localret, globalret;
#ifndef GLOBAL
    int provided;
    MPI_Init_thread ( &argc, &argv, MPI_THREAD_MULTIPLE, &provided );
    if ( argc > 1 ) sleep (20);
#endif

    MPI_Comm_size ( comm, &size );
    MPI_Comm_rank ( comm, &rank );

#ifndef GLOBAL
//    if ( size != 6 ) {
//	printf("Sorry, this test only works correctly with 6 processes\n");
//	MPI_Abort ( MPI_COMM_WORLD, 1);
//   }
#endif

    files = (MPI_File *) malloc (num_threads * sizeof (MPI_File));
    tids  = (pthread_t *) malloc (num_threads * sizeof (pthread_t));
    int ii;

    blength[0] = 6; 
    displ = rank * 6;
    MPI_Type_indexed (1, blength, &displ, MPI_INT, &fview );
    MPI_Type_commit ( &fview);

    for (ii=0; ii<num_threads; ii++ ) {
        char *filename;

        asprintf(&filename, "testfile-%d", ii);
        MPI_File_open (comm, filename, MPI_MODE_CREATE|MPI_MODE_WRONLY, 
                       MPI_INFO_NULL, &files[ii]);
        MPI_File_set_view ( files[ii], 0, MPI_INT, fview, "native", MPI_INFO_NULL );
    }

    if ( rank == root ) {
	printf("Checking MPI_File_write by threads on different handles...");
    }
    pthread_barrier_init (&barrier, NULL, num_threads+1);

    localret=0;
    for ( ii=0; ii<num_threads; ii++ ) {
        ret = pthread_create(&tids[ii], NULL, write_test, (void *)(long)num_threads );
        if ( ret != 0 ) {
#ifdef VERBOSE
            printf("rank:%d thread:master Could not spawn thread %d\n", rank, ii);
#endif
            localret = 1;
        }
    }
    pthread_barrier_wait ( &barrier);

    for ( ii=0; ii<num_threads; ii++ ) {
        long lret;
        pthread_join (tids[ii], (void *) &lret);
        if ( lret != 0 ) {
            printf("%d: Pthread join for thread %d returned %ld\n", rank, ii, lret );
            localret = 1;
        }
    }

    for ( ii=0; ii<num_threads; ii++ ) {
        MPI_File_close ( &files[ii]);

        if ( rank == root ) {
            int fh, veriarr[36];
            char *filename;
            asprintf(&filename, "testfile-%d", ii);            
	
            fh = open (filename, O_RDONLY );
            if ( -1 == fh ) {
                printf("Could not open file \n");
                MPI_Abort ( comm , -1 );
            }
            else {
                SL_read ( fh, veriarr, 6 * size *sizeof(int));
                for ( i=0; i< 36; i++ ) {
                    if (veriarr[i] != i ) {
                        localret = 1;
#ifdef VERBOSE
                        printf("File %s Element %d is %d\n", filename, i, veriarr[i]);
#endif
                    }
                }
            }
            close (fh);
            unlink(filename);            
        }
    }
    
  
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
    }


/**************************************************************************************************/
/**************************************************************************************************/
/**************************************************************************************************/
    if ( rank == root ) {
	printf("Checking MPI_File_write_at by threads on same file........");
    }

    MPI_File_open (comm, "writefile-mt.out", MPI_MODE_CREATE|MPI_MODE_WRONLY, 
                   MPI_INFO_NULL, &files[0]);

    pthread_barrier_init (&barrier, NULL, num_threads+1);

    localret=0;
    for ( ii=0; ii<num_threads; ii++ ) {
        ret = pthread_create(&tids[ii], NULL, write_at_test, (void *)(long)num_threads );
        if ( ret != 0 ) {
#ifdef VERBOSE
            printf("rank:%d thread:master Could not spawn thread %d\n", rank, ii);
#endif
            localret = 1;
        }
    }
    pthread_barrier_wait ( &barrier);

    for ( ii=0; ii<num_threads; ii++ ) {
        long lret;
        pthread_join (tids[ii], (void *) &lret);
        if ( lret != 0 ) {
            localret = 1;
        }
    }


    MPI_File_close ( &files[0]);

    if ( rank == root ) {
        int fh, *veriarr;
        char *filename;
        int totallen = size * num_threads * 6;
	
        veriarr = (int *) malloc ( totallen * sizeof(int));

        fh = open ("writefile-mt.out", O_RDONLY );
        if ( -1 == fh ) {
            printf("Could not open file \n");
            MPI_Abort ( comm , -1 );
        }
        else {
            SL_read ( fh, veriarr,  totallen* sizeof(int));
                for ( i=0; i< totallen; i++ ) {
                    if (veriarr[i] != i ) {
                        localret = 1;
#ifdef VERBOSE
                        printf("File %s Element %d is %d\n", filename, i, veriarr[i]);
#endif
                    }
                }
        }
        close (fh);
        unlink("writefile-mt.out");            

        free ( veriarr );
    }

    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
    }



    free ( files );

#ifndef GLOBAL
    MPI_Finalize ();
#endif

  return 0;
}
