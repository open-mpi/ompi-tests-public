#ifndef __ompio_cuda_utils_h__
#define __ompio_cuda_utils_h__

#include <string.h>
#include "mpi.h"


void SL_read ( int hdl, void *buf, int num );
void SL_write ( int hdl, void *buf, int num );
void SL_create_input_file ( int rank, const char *filename, int size, int offset );
int SL_verify_result_file ( int rank, const char *filename, int size, int offset );
int SL_verify_result_memory (int rank, int *writearr, int size, int offset );

#define ENV_LOCAL_RANK "OMPI_COMM_WORLD_LOCAL_RANK"
#define MAXLEN 12582912

/* Some macros for the Type_create_subarray test */
#define NG  1
#define NDIMS  4
#define NA  (size_t)256
#define NB  (size_t)128
#define NC  (size_t)128
#define ND  (size_t)32


#ifndef __CUDACC__
#define cudaError_t int
#define cudaSuccess 1
#define cudaMemcpyDeviceToHost 10
#define cudaMemcpyHostToDevice 11

cudaError_t cudaMallocManaged( int **buf, size_t len );
cudaError_t cudaMalloc( int **buf, size_t len );
cudaError_t cudaMemcpy( void *devicebuf, void *hostbuf, size_t len, int direction);
cudaError_t cudaMemset( void *buf, int value, size_t len );
cudaError_t cudaDeviceSynchronize();
cudaError_t cudaFree(int**);
cudaError_t cudaGetDeviceCount(int*);
cudaError_t cudaSetDevice(int);

#endif

#ifdef __CUDACC__
__global__ void add_one ( int *a, int num_elems );
#else
void add_one ( int *a, int num_elems );
#endif

void performCudaOperations (int *array, int arraylen );

#endif /*  __ompio_cuda_utils_h__ */


