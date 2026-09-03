/*Partitions MPI Unit Test
 *
 * Shows the behavior of the communication when Pready is call on an already active partition.
 * This action should be erroneous.
 */


#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "assert.h"

#define PARTITIONS 8
#define COUNT 5

int main (int argc, char *argv[])
{
//Buffer message
double message [PARTITIONS * COUNT];

//Defining an array of partitions 
int partitionsList[8];

//MPI varaibles declarations
int src = 0, dest = 1, tag = 100, flag = 0;
int myrank, provided, i, j;
MPI_Count partitions = PARTITIONS;
MPI_Request request;

//Initializing threaded MPI
MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
if (provided < MPI_THREAD_SERIALIZED)
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

if (myrank == 0)
{
        MPI_Psend_init(message, partitions, COUNT, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &request);
        MPI_Start(&request);

	//Iterating through each buffer partition, filling them and marking them ready to send
        for (i = 0; i < partitions; i++)
        {
                for (j = (i*COUNT); j < ((i+1)*COUNT); j++)
                {
                        message[j] = j+1;
                }
                partitionsList[i] = i;
		MPI_Pready(i, request);
        }
	
	//Mark ready  all of the partitions, which are already declared ready/active 
	MPI_Pready_list(partitions, partitionsList, request);

	//Test for overall send operation completion
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

	//Test for overall recieve operation completion
        while (!flag)
        {
                MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
        }
	
	//Test the buffer to check that the message was recieved correctly
        for (i = 0; i < (PARTITIONS * COUNT); i++)
	{
        	assert(message[i] == (i+1));
	}
	printf("Test Passed Succesfully\n");
        MPI_Request_free(&request);
}
MPI_Finalize();
return 0;
}

