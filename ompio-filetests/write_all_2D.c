/* This is a testcase based on a bug report exposing a problem in the dynamic_gen2 component.
** At the core, the problem is a 2-D data distribution resulting in an uneven distribution of data
** to the aggregators. 
**
** It used to be fully exposed witht mca io_ompio_num_aggregators 2 and mca io_ompio_bytes_per_agg 65536 
*/  

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "mpi.h"

#define NX 2048
#define NY 32
#define NNY 4

/* 2D array allocation */
static int malloc2D(int ***array, int n, int m) {
  int i;
  /* allocate the n*m contiguous items */
  int *p = malloc(n*m*sizeof(int));
  if (!p) return -1;

  /* allocate the row pointers into the memory */
  (*array) = malloc(n*sizeof(int*));
  if (!(*array)) {
    free(p);
    return -1;
  }

  /* set up the pointers into the contiguous memory */
  for (i=0; i<n; i++)
    (*array)[i] = &(p[i*m]);

  return 0;
}

static int free2D(int ***array) {
  /* free the memory - the first element of the array is at the start */
  free(&((*array)[0][0]));

  /* free the pointers into the memory */
  free(*array);

  return 0;
}


int write_all_2D (MPI_Comm comm, int root ) 
{
  int total = 0;

  int i, j, ret;
  int nx , ny;
  int my_rank;
  int nproc;
  int period[2] = {0, 0};
  int coord[2];
  int reorder = 0;
  int **local = NULL;
  MPI_Comm grid_Comm;
  MPI_Status status;
  int dim[2] = {2, 3};
  MPI_File fpc;


  /* Initialize MPI */
  MPI_Comm_size(comm, &nproc);
  MPI_Comm_rank(comm, &my_rank);

  if ( nproc != 6 ) {
      printf("Sorry, this test only works for 6 processes correctly.\n");
      MPI_Abort ( comm,1 );
  }
  /* Create the file with all values set to -1 */
  if ( my_rank == 0 ) {
      printf("    2-D pattern w/ uneven distribution.....");

      int *dummy = (int *) malloc ( NX * NY * nproc * sizeof(int));
      for ( i=0; i< (NX*NY*nproc); i++ ) {
          dummy[i] = -1;
      }
      MPI_File_open(MPI_COMM_SELF, "d.mat", MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &fpc);
      MPI_File_write (fpc, dummy, NX*NY*nproc, MPI_INT, &status);
      MPI_File_close ( &fpc);
      free ( dummy );
  }
  
  sync();

  /* Establish cartesian topology */
  MPI_Cart_create(comm, 2, dim, period, reorder, &grid_Comm);

  /* Get cartesian grid indicies of processes */
  MPI_Cart_coords(grid_Comm, my_rank, 2, coord);
  malloc2D(&local,NX,NY);

  /* Create derived type for file view */
  MPI_Datatype view;
  int startV[2] = { coord[0]*NX, coord[1]*NY };
  int arrsizeV[2] = { dim[0]*NX, dim[1]*NY };
  int gridsizeV[2] = { NX, NY };

#ifdef DEBUG
  printf("%d: coords[%d][%d] start[%d][%d] size[%d][%d] gridsize[%d][%d]\n", my_rank, coord[0], coord[1], coord[0]*NX, coord[1]*NY, 
         dim[0]*NX, dim[1]*NY, NX, NY);
#endif

  MPI_Type_create_subarray(2, arrsizeV, gridsizeV, startV, MPI_ORDER_C, MPI_INT, &view);
  MPI_Type_commit(&view);

  for (i=0; i< NX; i++) {
      int globalrow = coord[0]*NX + i;
      for ( j=0; j<NY; j++) {
          int globalcolumn = coord[1]*NY + j;
          local[i][j]=globalrow*NY*dim[1] + globalcolumn;
      }
  }
  
  /* MPI IO */
  MPI_File_open(grid_Comm, "d.mat", MPI_MODE_WRONLY , MPI_INFO_NULL, &fpc);
  MPI_File_set_view(fpc, 0, MPI_INT, view, "native", MPI_INFO_NULL);
  ret = MPI_File_write_all(fpc, &(local[0][0]), NX*NNY, MPI_INT, &status);
  if ( MPI_SUCCESS != ret ) {
      printf("Error in File_write_all\n" );
  }
#ifdef DEBUG
  int count;
  MPI_Get_elements ( &status, MPI_INT, &count );
  printf("count = %d should be %d\n", count, (NX*NNY*nproc));
#endif
  MPI_File_close( &fpc );

  MPI_Type_free(&view);
  MPI_Comm_free ( &grid_Comm);
  free2D(&local);
  sync();

  
  /* Verification of the file */
  if ( my_rank == 0 ) {
      int ii,jj, should_be;
      int working =1; //true

      malloc2D(&local,NX*dim[0],NY*dim[1]);
      MPI_File_open(MPI_COMM_SELF, "d.mat", MPI_MODE_RDONLY|MPI_MODE_DELETE_ON_CLOSE, MPI_INFO_NULL, &fpc);
      ret = MPI_File_read(fpc, &(local[0][0]), NX*NY*nproc, MPI_INT, &status);
      if ( MPI_SUCCESS != ret ) {
          printf("MPI_File_read: error =%d\n", ret );
      }

      MPI_File_close ( &fpc);

      for ( ii=0; ii< dim[0]; ii++ ) {
          for ( jj=0; jj<dim[1]; jj++ ) {

              for ( i=0; i<NX; i++ ) {
                  for ( j=0; j< NY; j++ ) {

                      if ( (i*NY+j) < (NX*NNY) ) {
                          should_be = (ii*NX+i)*NY*dim[1] + (jj*NY+j);
                          if ( local[ii*NX+i][jj*NY+j] != should_be ) {
#ifdef DEBUG
                              printf("coords[%d][%d] i=%d j=%d value %d should be %d\n", ii,jj, i, j, local[ii*NX+i][jj*NY+j], 
                                     should_be);
#endif
                              working=0;
                          } 
                      }
                      else {
                          if ( local[ii*NX+i][jj*NY+j] != -1 ) {
#ifdef DEBUG
                              printf("coords[%d][%d] i=%d j=%d value %d should be -1\n", ii,jj, i, j, local[ii*NX+i][jj*NY+j]);
#endif
                              working=0;
                          }
                      }
                  }
              }
          }
      }
      if ( working ) {
          printf("working\n");
      }
      else {
          printf("false\n");
          total++;
      }
  }

  return total;
}