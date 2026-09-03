/*
** This file test MPI_File_open and close. 
*/
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "mpi.h"

static void *open_test( void* arg);

MPI_File *files=NULL;
MPI_Comm *comms=NULL;
pthread_t *tids = NULL;
int rank, size;
pthread_barrier_t   barrier; // barrier synchronization object


static void * open_test ( void *arg )
{
    int ret= MPI_SUCCESS;
    long myret = 0;
    pthread_t t_id = pthread_self();
    char *filename;
    int i, max_num_threads = (int) ((long)arg);
    
    pthread_barrier_wait( &barrier );
    for (i=0; i< max_num_threads; i++ ) {
        if ( tids[i] == t_id ) break;
    }
    asprintf(&filename, "testfile-%d", i);

#ifdef VERBOSE
    printf("%d / %d: before File_open of file %s\n", rank, i, filename);
#endif
    ret = MPI_File_open ( comms[i], filename, MPI_MODE_CREATE|MPI_MODE_WRONLY, 
			  MPI_INFO_NULL,  &files[i] );
    if ( MPI_SUCCESS != ret ) {
#ifdef VERBOSE
        printf("rank:%d thread:%d error in File_open, error code %d\n", rank, i, ret);
#endif
        myret = 1;
    }
#ifdef VERBOSE    
    printf("%d / %d: after File_open of file %s\n", rank, i, filename);
#endif
    pthread_exit ((void *) myret);
    return NULL;
}

#ifdef GLOBAL
int mt_open_test ( MPI_Comm comm, int num_threads, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
    int num_threads=3;
#endif

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

    comms = (MPI_Comm *) malloc (num_threads * sizeof (MPI_Comm));
    files = (MPI_File *) malloc (num_threads * sizeof (MPI_File));
    tids  = (pthread_t *) malloc (num_threads * sizeof (pthread_t));
    int ii;
    for (ii=0; ii<num_threads; ii++ ) {
        MPI_Comm_dup ( comm, &comms[ii]);
    }

    if ( rank == root ) {
	printf("Checking for MPI_File_open by many threads................");
    }

    pthread_barrier_init (&barrier, NULL, num_threads+1);

    localret=0;
    for ( ii=0; ii<num_threads; ii++ ) {
        ret = pthread_create(&tids[ii], NULL, open_test, (void *)(long)num_threads );
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
  
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
    }


    for ( ii=0; ii<num_threads; ii++ ) {
        char *filename;

        MPI_File_close ( &files[ii]);
        MPI_Comm_free ( &comms[ii]);
        
        if ( rank == root ) {
            asprintf(&filename, "testfile-%d", ii);
            unlink(filename);
        }
    }

#ifndef GLOBAL
    MPI_Finalize ();
#endif

    return 0;
}




