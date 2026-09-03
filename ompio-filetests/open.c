/*
** This file test MPI_File_open and close. 
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mpi.h"




int cart_comm_open_2D( MPI_Comm comm, int root );
int cart_comm_open_3D( MPI_Comm comm, int root );

#ifdef GLOBAL
int open_test ( MPI_Comm comm, int root )
{
#else
int main ( int argc, char * argv[] )
{
    MPI_Comm comm=MPI_COMM_WORLD;
    int root=0;
#endif

    int fd, rank, size, i, ret, ret2, localret, globalret, closeret;
    int total = 0;
    MPI_File file1=MPI_FILE_NULL, file2=MPI_FILE_NULL;

    
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


    if ( rank == root ) {
	printf("Checking for MPI_File_open:\n");
	printf("    using MPI_MODE_CREATE..................");
    }

    ret = MPI_File_open ( comm, "testfile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY, 
			  MPI_INFO_NULL,  &file1 );
  
    if ( ret == MPI_SUCCESS && file1 != MPI_FILE_NULL ) 
	localret = 0;
    else
	localret = 1;
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root )  {
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	total += globalret;
    }

    closeret = MPI_File_close ( &file1 );

   /**********************************************************************/ 
    if ( rank == root ) {
	printf("    using MPI_MODE_WRONLY..................");
    }

    ret = MPI_File_open ( comm, "testfile1.out", MPI_MODE_WRONLY, MPI_INFO_NULL,
			  &file1 );
  
    if ( ret == MPI_SUCCESS && file1 != MPI_FILE_NULL ) 
	localret = 0;
    else
	localret = 1;
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root )  {
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	total += globalret;
    }
    MPI_File_close ( &file1 );

    /**********************************************************************/
    if ( rank == root ) {
	printf("    using MPI_MODE_RDONLY..................");
    }

    ret = MPI_File_open ( comm, "testfile1.out", MPI_MODE_RDONLY, MPI_INFO_NULL,
			  &file1 );
  
    if ( ret == MPI_SUCCESS && file1 != MPI_FILE_NULL ) 
	localret = 0;
    else
	localret = 1;
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root )  {
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	total += globalret;
    }
    MPI_File_close ( &file1 );

    /**********************************************************************/
    if ( rank == root ) {
	printf("    using MPI_MODE_RDWR....................");
    }

    ret = MPI_File_open ( comm, "testfile1.out", MPI_MODE_RDWR, MPI_INFO_NULL,
			  &file1 );
  
    if ( ret == MPI_SUCCESS && file1 != MPI_FILE_NULL ) 
	localret = 0;
    else
	localret = 1;
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root )  {
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	total += globalret;
    }
    MPI_File_close ( &file1 );

    /**********************************************************************/
    if ( rank == root ) {
	printf("    using MPI_MODE_APPEND..................");
    }

    ret = MPI_File_open ( comm, "testfile1.out", MPI_MODE_APPEND|MPI_MODE_WRONLY, MPI_INFO_NULL,
			  &file1 );
  
    if ( ret == MPI_SUCCESS && file1 != MPI_FILE_NULL ) 
	localret = 0;
    else
	localret = 1;
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root )  {
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	total += globalret;
    }
    MPI_File_close ( &file1 );

    /**********************************************************************/
    if ( rank == root ) {
	printf("    using MPI_MODE_EXCL....................");
    }

    ret = MPI_File_open ( comm, "testfile1.out", MPI_MODE_EXCL|MPI_MODE_CREATE|MPI_MODE_WRONLY, 
			  MPI_INFO_NULL,   &file1 );
  
    if ( ret != MPI_SUCCESS && file1 == MPI_FILE_NULL ) 
	localret = 0;
    else
	localret = 1;
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);
    
    if ( rank == root )  { 
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
    }

    /**********************************************************************/ 
    if ( rank == root ) {
	printf("    using MPI_MODE_DELETE_ON_CLOSE.........");
    }

    ret = MPI_File_open ( comm, "testfile1.out", MPI_MODE_DELETE_ON_CLOSE|MPI_MODE_WRONLY, 
			  MPI_INFO_NULL,   &file1 );
    MPI_File_close ( &file1 );

    ret2 = MPI_File_open ( comm, "testfile1.out", MPI_MODE_RDONLY, MPI_INFO_NULL, &file2 );

    if ( ret == MPI_SUCCESS && ret2 != MPI_SUCCESS && file2 == MPI_FILE_NULL ) 
	localret = 0;
    else
	localret = 1;
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root )  {
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	total += globalret;
    }

    /**********************************************************************/
    cart_comm_open_2D ( comm, root );
    /**********************************************************************/ 
    cart_comm_open_3D ( comm, root );
    /**********************************************************************/ 
    /* Error code verification */
    if ( rank == root ) {
	printf("Checking for Error codes with File_open:\n");
	printf("    MPI_ERR_NO_SUCH_FILE...................");
    }

    ret = MPI_File_open ( comm, "testfile-does-not-exist.out", MPI_MODE_RDONLY, MPI_INFO_NULL,
			  &file1 );

    if ( ret == MPI_ERR_NO_SUCH_FILE ) 
	localret = 0;
    else
	localret = 1;

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root )  {
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	total += globalret;
    }

    /* Error code verification */
    if ( rank == root ) {
	printf("    MPI_ERR_ACCESS.........................");

        fd = open("writefile1.out", O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU);
        write ( fd, &rank, sizeof(int));
        fsync (fd);
        close (fd);
        chmod ("writefile1.out", S_IRUSR | S_IRGRP );
    }

    MPI_Barrier ( comm );
    ret = MPI_File_open ( comm, "writefile1.out", MPI_MODE_WRONLY, MPI_INFO_NULL,
			  &file1 );

    if ( ret == MPI_ERR_ACCESS ) 
	localret = 0;
    else
	localret = 1;

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root )  {
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	total += globalret;
    }

    /* Error code verification */
    if ( rank == root ) {
	printf("    MPI_ERR_FILE_EXISTS....................");
    }

    MPI_Barrier ( comm );
    ret = MPI_File_open ( comm, "writefile1.out", MPI_MODE_RDWR|MPI_MODE_CREATE|MPI_MODE_EXCL, MPI_INFO_NULL,
			  &file1 );

    if ( ret == MPI_ERR_FILE_EXISTS ) 
	localret = 0;
    else
	localret = 1;

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root )  {
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	total += globalret;
        unlink ("writefile1.out");
    }


    /**********************************************************************/
    /* We used file close multiple times already, use however the return 
       value of the very first MPI_File_close operation */
    if ( rank == root ) {
	printf("Checking for MPI_File_close................");
    }

    if ( closeret == MPI_SUCCESS ) 
	localret = 0;
    else
	localret = 1;

    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root )  {
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	total += globalret;
    }
    /**********************************************************************/
    if ( rank == root ) {
	printf("Checking File open/close 2048 times........");
    }

    for ( i=0; i<2048; i++ ) {
        ret = MPI_File_open ( comm, "testfile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY, 
                        MPI_INFO_NULL, &file1 );
        ret2 = MPI_File_close ( &file1 );
    }
    if ( ret == MPI_SUCCESS  && ret2 == MPI_SUCCESS ) 
        localret = 0;
    else
	localret = 1;
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root )  {
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	total += globalret;
    }


    /**********************************************************************/
    if ( rank == root ) {
	printf("Checking for MPI_File_delete...............");
    }

    MPI_File_open ( comm, "testfile1.out", MPI_MODE_CREATE|MPI_MODE_WRONLY, 
		    MPI_INFO_NULL, &file1 );
    if ( rank == root ) {
	MPI_File_write ( file1, &root, 1, MPI_INT, MPI_STATUS_IGNORE );
    }
    MPI_Barrier ( comm );
    MPI_File_close ( &file1 );
    
    ret = MPI_File_delete ( "testfile1.out", MPI_INFO_NULL );
    ret2 = MPI_File_open ( comm, "testfile1.out", 
			   MPI_MODE_WRONLY|MPI_MODE_EXCL|MPI_MODE_CREATE|MPI_MODE_DELETE_ON_CLOSE, 
			   MPI_INFO_NULL,   &file1 );
    MPI_File_close ( &file1 );
    
    if ( (ret == MPI_SUCCESS || ret == MPI_ERR_NO_SUCH_FILE) && ret2 == MPI_SUCCESS ) 
	localret = 0;
    else
	localret = 1;
    
    MPI_Reduce ( &localret, &globalret, 1, MPI_INT, MPI_MAX, root, comm);

    if ( rank == root )  {
	if ( globalret == 0 )
	    printf("working\n");
	else
	    printf("false\n");
	total += globalret;
    }


#ifndef GLOBAL
  MPI_Finalize ();
#endif

  return total;
}




