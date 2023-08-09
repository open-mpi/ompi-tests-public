#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "assert.h"

#define PARTITIONS 8
#define COUNT 5

int main (int argc, char *argv[])
{
double numbers [PARTITIONS * COUNT];
double sum [PARTITIONS * COUNT];
int src = 0, dest = 1, tag = 100, sumtag = 101, flag = 0, flag2 = 0;
int myrank, provided, len, i, j;
char hostname[MPI_MAX_PROCESSOR_NAME];
MPI_Count partitions = PARTITIONS;
MPI_Request requests[2];

MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
if (provided < MPI_THREAD_SERIALIZED)
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
MPI_Get_processor_name(hostname, &len);

if (myrank == 0)
{
        MPI_Psend_init(numbers, partitions, COUNT, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &requests[0]);
	MPI_Psend_init(sum, partitions, COUNT, MPI_DOUBLE, dest, sumtag, MPI_COMM_WORLD, MPI_INFO_NULL, &requests[1]);

        MPI_Startall(2, requests);

        for (i = 0; i < partitions; i++)
        {
                for (j = (i*COUNT); j < ((i+1)*COUNT); j++)
                {
                        numbers[j] = j+1;
			sum[j] = numbers[j] + sum[j-1]; 
                }
                MPI_Pready(i, requests[0]);
		MPI_Pready(i, requests[1]);
        }

        while (!flag || !flag2)
        {
                MPI_Test(&requests[0], &flag, MPI_STATUS_IGNORE);
                MPI_Test(&requests[1], &flag2, MPI_STATUS_IGNORE);
        }
        MPI_Request_free(&requests[0]);
        MPI_Request_free(&requests[1]);
}
else if (myrank == 1)
{
        MPI_Precv_init(numbers, partitions, COUNT, MPI_DOUBLE, src, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &requests[0]);
	MPI_Precv_init(sum, partitions, COUNT, MPI_DOUBLE, src, sumtag, MPI_COMM_WORLD, MPI_INFO_NULL, &requests[1]);
        
	MPI_Startall(2, requests);

        while (!flag || !flag2)
        {
                MPI_Test(&requests[0], &flag, MPI_STATUS_IGNORE);
		MPI_Test(&requests[1], &flag2, MPI_STATUS_IGNORE);
        }
        MPI_Request_free(&requests[0]);
	MPI_Request_free(&requests[1]);
	
	for (i = 0; i < (PARTITIONS * COUNT); i++)
        {
                assert(numbers[i] = (i+1));
		assert(sum[j] = numbers[j] + sum[j-1]);
        }	

	printf("Test Passed Succesfully\n");
}
MPI_Finalize();
return 0;
}

