/*Partitions MPI Unit Test
 *
 * Shows the behavior of the communication with 2 send / recv corridors initialied.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "assert.h"

#define PARTITIONS 8
#define COUNT 5

int main (int argc, char *argv[])
{
//Buffer messages
double numbers [PARTITIONS * COUNT];
double sum [PARTITIONS * COUNT];

//MPI variables declarations
int src = 0, dest = 1, tag = 100, sumtag = 101, flag = 0, flag2 = 0;
int myrank, provided, i, j;

MPI_Count partitions = PARTITIONS;
MPI_Request requests[2];

//Initializing threaded MPI
MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
if (provided < MPI_THREAD_SERIALIZED)
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

if (myrank == 0)
{
        MPI_Psend_init(numbers, partitions, COUNT, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &requests[0]);
	MPI_Psend_init(sum, partitions, COUNT, MPI_DOUBLE, dest, sumtag, MPI_COMM_WORLD, MPI_INFO_NULL, &requests[1]);

        MPI_Startall(2, requests);

	//Iterating through each buffer partition, filling them and marking them ready to send
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

	//Test for overall send operation completion
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

	//Test for overall send operation completion
        while (!flag || !flag2)
        {
                MPI_Test(&requests[0], &flag, MPI_STATUS_IGNORE);
		MPI_Test(&requests[1], &flag2, MPI_STATUS_IGNORE);
        }
        MPI_Request_free(&requests[0]);
	MPI_Request_free(&requests[1]);
	
	//Test the buffer to check that the message was recieved correctly
	for (i = 0; i < (PARTITIONS * COUNT); i++)
        {
                assert(numbers[i] == (i+1));
		assert(sum[j] == numbers[j] + sum[j-1]);
        }	

	printf("Test Passed Succesfully\n");
}
MPI_Finalize();
return 0;
}

