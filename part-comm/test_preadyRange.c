/*Partitions MPI Unit Test
 *
 * Shows the behavior of the communication when partitions are marked ready through Pready_range.
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

//MPI variables declarations 
int src = 0, dest = 1, tag = 100, flag = 0;
int myrank, provided, i, j;
MPI_Count partitions = PARTITIONS;
MPI_Request request;

//Initializing threaded MPI
MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
if (provided < MPI_THREAD_SERIALIZED)
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

//For this test 4 or more partitions are necessary
if (partitions < 4)
{
	printf("Partitions must be greater or equal to 4, current number of partitions: %d", partitions);
	MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
}

if (myrank == 0)
{
        MPI_Psend_init(message, partitions, COUNT, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &request);
        MPI_Start(&request);

	//Iterating through each buffer partition and filling them
        for (i = 0; i < partitions; i++)
        {
                for (j = (i*COUNT); j < ((i+1)*COUNT); j++)
                {
                        message[j] = j+1;
                }
		//Marks the first half of partitions reday to send 
		if ( i = ((partitions/2)-1) ){
			MPI_Pready_range(0, ((partitions/2)-1), request);
		}
		//Marks the second half of partitions ready to send
		else if ( i = (partitions-1) ){
			MPI_Pready_range((partitions/2), (partitions-1), request);
		}
        }

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

