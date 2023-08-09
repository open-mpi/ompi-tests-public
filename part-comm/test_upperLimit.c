#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "assert.h"

#define PARTITIONS 1

int main (int argc, char *argv[])
{
char *count = argv[1];
int COUNT = 1040000 + atoi(count);
double message [PARTITIONS * COUNT];
int src = 0, dest = 1, tag = 100, flag = 0, flag2 = 0;
int myrank, provided, len, i, j;
char hostname[MPI_MAX_PROCESSOR_NAME];
MPI_Count partitions = PARTITIONS;
MPI_Request request;

MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
if (provided < MPI_THREAD_SERIALIZED)
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
MPI_Get_processor_name(hostname, &len);

if (myrank == 0)
{
        MPI_Psend_init(message, partitions, COUNT, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &request);
        MPI_Start(&request);

        for (i = 0; i < partitions; i++)
        {
                for (j = (i*COUNT); j < ((i+1)*COUNT); j++)
                {
                        message[j] = j+1;
                }
                MPI_Pready(i, request);
        }
        while (!flag)
        {
                MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
        }
        MPI_Request_free(&request);
}
else if (myrank == 1)
{
        MPI_Precv_init(message, partitions, COUNT, MPI_DOUBLE, src, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &request);
        MPI_Start(&request);

        while (!flag)
        {
                MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
        }

        for (i = 0; i < (PARTITIONS * COUNT); i++)
	{
        	assert(message[i] = (i+1));
	}
	printf("\n Test %d Passed Succesfully\n", COUNT);
        MPI_Request_free(&request);
}
MPI_Finalize();
return 0;
}

