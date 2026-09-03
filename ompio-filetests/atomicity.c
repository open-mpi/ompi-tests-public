/*
** This file test the atomicity operations of MPI.
** The test is based on the same test from the mpich 
** testsuites, expands on it however to also
** use non-blocking operations (knowing well that
** we might not pass that for a while).
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mpi.h"


#define BUFSIZE 10000
//#define BUFSIZE 10

#ifdef GLOBAL
int atomicity_test ( MPI_Comm comm, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
#endif

    int rank, size, i, ret,  localret, globalret;
    MPI_File file1=MPI_FILE_NULL;
    MPI_Status status;
    MPI_Datatype newtype;
    MPI_Request req;
    int *writebuf=NULL, *readbuf=NULL;
    
#ifndef GLOBAL
    MPI_Init ( &argc, &argv );
    if ( argc > 1 ) sleep (20);
#endif

    MPI_Comm_size ( comm, &size );
    MPI_Comm_rank ( comm, &rank );

#ifndef GLOBAL

//    if ( size != 6 ) {
//       printf("Sorry, this test only works correctly with 6 processes\n");
//	MPI_Abort ( MPI_COMM_WORLD, 1);
//    }
#endif

    writebuf = (int *) malloc(BUFSIZE * sizeof(int));
    readbuf = (int *) malloc(BUFSIZE * sizeof(int));
    if ( NULL == writebuf || NULL==readbuf ) {
        return -1;
    }

    MPI_Type_vector(BUFSIZE, 1, 2, MPI_INT, &newtype);
    MPI_Type_commit(&newtype);
    
/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
    if ( rank == root ) {
	printf("Checking for MPI_File_set_atomicity:\n");
	printf("    contig. data, blocking I/O.............");
    }

    if ( rank == root ) {
        MPI_File_open ( MPI_COMM_SELF, "testfile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY, 
                        MPI_INFO_NULL,  &file1 );
        for (i = 0; i < BUFSIZE; i++)
            writebuf[i] = 0;
        
        MPI_File_write(file1, writebuf, BUFSIZE, MPI_INT, &status);
        MPI_File_close(&file1);
    }
    MPI_Barrier (comm);

    for (i = 0; i < BUFSIZE; i++)
        writebuf[i] = 10;
    for (i = 0; i < BUFSIZE; i++)
        readbuf[i] = 20;

    MPI_File_open( comm, "testfile1.out", MPI_MODE_RDWR, MPI_INFO_NULL,  &file1 );

    /* set atomicity to true */
    ret = MPI_File_set_atomicity(file1, 1);
    if (ret != MPI_SUCCESS) {
        if ( rank == root ) 
            printf("\n Atomic mode not supported on this file system.\n");
        return -1;
    }

    MPI_Barrier(comm);

    /* process 0 writes and others concurrently read. In atomic mode,
    ** the data read must be either all old values or all new values; nothing
    ** in between. 
    */
    int num_elems;
    if (rank == root ) {
        localret = MPI_File_write(file1, writebuf, BUFSIZE, MPI_INT, &status);
    }
    else {
        ret = MPI_File_read(file1, readbuf, BUFSIZE, MPI_INT, &status);
        localret = 0;
        if ( ret == MPI_SUCCESS) {
            if (readbuf[0] == 0) {      /* the rest must also be 0 */
                for (i = 1; i < BUFSIZE; i++) {
                    if (readbuf[i] != 0) {
#ifdef VERBOSE
                        printf("[%d]: readbuf[%d] is %d, should be 0\n", rank, i,
                               readbuf[i]);
#endif
                        localret = 1;
                    }
                }
            }
            else if (readbuf[0] == 10) {      /* the rest must also be 10 */
                for (i = 1; i < BUFSIZE; i++) {
                    if (readbuf[i] != 10) {
#ifdef VERBOSE
                        printf("[%d]: readbuf[%d] is %d, should be 10\n", rank, i,
                               readbuf[i]);
#endif
                        localret = 1;
                    }
                }
            }
            else {
                localret = 1;
#ifdef VERBOSE
                printf("[%d]: readbuf[0] is %d, should be either 0 or 10\n", rank,
                       readbuf[0]);
#endif
            }
        }
    }
    
    MPI_File_close(&file1);
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
        unlink("testfile1.out");
    }

/**************************************************************************************/
/**************************************************************************************/
/**************************************************************************************/

    if ( rank == root ) {
	printf("    non-contig. data, blocking I/O.........");
    }

    if ( rank == root ) {
        MPI_File_open ( MPI_COMM_SELF, "testfile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY, 
                        MPI_INFO_NULL,  &file1 );
        for (i = 0; i < BUFSIZE; i++)
            writebuf[i] = 0;

        MPI_File_set_view(file1, 0, MPI_INT, newtype, "native", MPI_INFO_NULL);
        MPI_File_write(file1, writebuf, BUFSIZE, MPI_INT, &status);
        int num;
        MPI_Get_elements ( &status, MPI_INT, &num );
        if ( num != BUFSIZE ) {
            printf("[%d]: didn't write enough bytes num=%d\n", rank, num);
        }
        MPI_File_close(&file1);
    }
    MPI_Barrier (comm);

    for (i = 0; i < BUFSIZE; i++)
        writebuf[i] = 10;
    for (i = 0; i < BUFSIZE; i++)
        readbuf[i] = 20;

    MPI_File_open( comm, "testfile1.out", MPI_MODE_RDWR, MPI_INFO_NULL,  &file1 );
    
    /* set atomicity to true */
    ret = MPI_File_set_atomicity(file1, 1);
    if (ret != MPI_SUCCESS) {
        if ( rank == root ) 
            printf("\n Atomic mode not supported on this file system.\n");
        return -1;
    }

    MPI_File_set_view(file1, 0, MPI_INT, newtype, "native", MPI_INFO_NULL);
    MPI_Barrier(comm);

    /* process 0 writes and others concurrently read. In atomic mode,
    ** the data read must be either all old values or all new values; nothing
    ** in between. 
    */
    int num_elements;
    if (rank == root ) {
        localret = MPI_File_write(file1, writebuf, BUFSIZE, MPI_INT, &status);
        MPI_Get_elements ( &status, MPI_INT, &num_elements);
    }
    else {
        ret = MPI_File_read(file1, readbuf, BUFSIZE, MPI_INT, &status);
        MPI_Get_elements ( &status, MPI_INT, &num_elements);
        localret = 0;
        if ( ret == MPI_SUCCESS) {
            if (readbuf[0] == 0) {      /* the rest must also be 0 */
                for (i = 1; i < BUFSIZE; i++) {
                    if (readbuf[i] != 0) {
#ifdef VERBOSE
                        printf("[%d]: readbuf[%d] is %d, should be 0\n", rank, i,
                               readbuf[i]);
#endif
                        localret = 1;
                    }
                }
            }
            else if (readbuf[0] == 10) {      /* the rest must also be 10 */
                for (i = 1; i < BUFSIZE; i++) {
                    if (readbuf[i] != 10) {
#ifdef VERBOSE
                        printf("[%d]: readbuf[%d] is %d, should be 10\n", rank, i,
                               readbuf[i]);
#endif
                        localret = 1;
                    }
                }
            }
            else {
                localret = 1;
#ifdef VERBOSE
                printf("[%d]: readbuf[0] is %d, should be either 0 or 10\n", rank,
                       readbuf[0]);
#endif
            }
        }
    }

    MPI_File_close(&file1);
    MPI_Barrier ( comm );
    
//    printf("[%d]: localret = %d numelements %d readbuf[0] = %d readbuf[BUFSIZE-1] = %d\n",
//           rank, localret, num_elements, readbuf[0], readbuf[BUFSIZE-1]);
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");

        unlink("testfile1.out");
    }

/**************************************************************************************/
/**************************************************************************************/
/**************************************************************************************/

    if ( rank == root ) {
	printf("    contig. data, non-blocking I/O.........");
    }

    if ( rank == root ) {
        MPI_File_open ( MPI_COMM_SELF, "testfile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY, 
                        MPI_INFO_NULL,  &file1 );
        for (i = 0; i < BUFSIZE; i++)
            writebuf[i] = 0;
        
        MPI_File_write(file1, writebuf, BUFSIZE, MPI_INT, &status);
        MPI_File_close(&file1);
    }
    MPI_Barrier (comm);

    for (i = 0; i < BUFSIZE; i++)
        writebuf[i] = 10;
    for (i = 0; i < BUFSIZE; i++)
        readbuf[i] = 20;

    MPI_File_open( comm, "testfile1.out", MPI_MODE_RDWR, MPI_INFO_NULL,  &file1 );

    /* set atomicity to true */
    ret = MPI_File_set_atomicity(file1, 1);
    if (ret != MPI_SUCCESS) {
        if ( rank == root ) 
            printf("\n Atomic mode not supported on this file system.\n");
        return -1;
    }

    MPI_Barrier(comm);

    /* process 0 writes and others concurrently read. In atomic mode,
    ** the data read must be either all old values or all new values; nothing
    ** in between. 
    */
    
    if (rank == root ) {
        localret = MPI_File_iwrite(file1, writebuf, BUFSIZE, MPI_INT, &req);
        MPI_Wait ( &req, &status);
    }
    else {
        ret = MPI_File_iread(file1, readbuf, BUFSIZE, MPI_INT, &req);
        MPI_Wait ( &req, &status);
        localret = 0;
        if ( ret == MPI_SUCCESS) {
            if (readbuf[0] == 0) {      /* the rest must also be 0 */
                for (i = 1; i < BUFSIZE; i++) {
                    if (readbuf[i] != 0) {
#ifdef VERBOSE
                        printf("[%d]: readbuf[%d] is %d, should be 0\n", rank, i,
                               readbuf[i]);
#endif
                        localret = 1;
                    }
                }
            }
            else if (readbuf[0] == 10) {      /* the rest must also be 10 */
                for (i = 1; i < BUFSIZE; i++) {
                    if (readbuf[i] != 10) {
#ifdef VERBOSE
                        printf("[%d]: readbuf[%d] is %d, should be 10\n", rank, i,
                               readbuf[i]);
#endif
                        localret = 1;
                    }
                }
            }
            else {
                localret = 1;
#ifdef VERBOSE
                printf("[%d]: readbuf[0] is %d, should be either 0 or 10\n", rank,
                       readbuf[0]);
#endif
            }
        }
    }

    MPI_File_close(&file1);

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");

        unlink("testfile1.out");
    }

    
/**************************************************************************************/
/**************************************************************************************/
/**************************************************************************************/
    if ( rank == root ) {
	printf("    non-contig. data, non-blocking I/O.....");
    }

    if ( rank == root ) {
        MPI_File_open ( MPI_COMM_SELF, "testfile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY, 
                        MPI_INFO_NULL,  &file1 );
        for (i = 0; i < BUFSIZE; i++)
            writebuf[i] = 0;
        
        MPI_File_set_view(file1, 0, MPI_INT, newtype, "native", MPI_INFO_NULL);
        MPI_File_write(file1, writebuf, BUFSIZE, MPI_INT, &status);
        MPI_File_close(&file1);
    }
    MPI_Barrier (comm);

    for (i = 0; i < BUFSIZE; i++)
        writebuf[i] = 10;
    for (i = 0; i < BUFSIZE; i++)
        readbuf[i] = 20;

    MPI_File_open( comm, "testfile1.out", MPI_MODE_RDWR, MPI_INFO_NULL,  &file1 );

    /* set atomicity to true */
    ret = MPI_File_set_atomicity(file1, 1);
    if (ret != MPI_SUCCESS) {
        if ( rank == root ) 
            printf("\n Atomic mode not supported on this file system.\n");
        return -1;
    }
    MPI_File_set_view(file1, 0, MPI_INT, newtype, "native", MPI_INFO_NULL);
    MPI_Barrier(comm);

    /* process 0 writes and others concurrently read. In atomic mode,
    ** the data read must be either all old values or all new values; nothing
    ** in between. 
    */
    
    if (rank == root ) {
        localret = MPI_File_iwrite(file1, writebuf, BUFSIZE, MPI_INT, &req);
        MPI_Wait ( &req, &status );
    }
    else {
        ret = MPI_File_iread(file1, readbuf, BUFSIZE, MPI_INT, &req);
        MPI_Wait ( &req, &status);
        localret = 0;
        if ( ret == MPI_SUCCESS) {
            if (readbuf[0] == 0) {      /* the rest must also be 0 */
                for (i = 1; i < BUFSIZE; i++) {
                    if (readbuf[i] != 0) {
#ifdef VERBOSE
                        printf("[%d]: readbuf[%d] is %d, should be 0\n", rank, i,
                               readbuf[i]);
#endif
                        localret = 1;
                    }
                }
            }
            else if (readbuf[0] == 10) {      /* the rest must also be 10 */
                for (i = 1; i < BUFSIZE; i++) {
                    if (readbuf[i] != 10) {
#ifdef VERBOSE
                        printf("[%d]: readbuf[%d] is %d, should be 10\n", rank, i,
                               readbuf[i]);
#endif
                        localret = 1;
                    }
                }
            }
            else {
                localret = 1;
#ifdef VERBOSE
                printf("[%d]: readbuf[0] is %d, should be either 0 or 10\n", rank,
                       readbuf[0]);
#endif
            }
        }
    }

    MPI_File_close(&file1);

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
        unlink("testfile1.out");
    }

    

#ifndef GLOBAL
  MPI_Finalize ();
#endif

  return 0;
}




