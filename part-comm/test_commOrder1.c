/*Partitions MPI Unit Test
 *  
 * Shows the behavior of the communicattion when a partitioned send call completes 
 * before a partitioned recieve call is declared
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

//MPI variable declaration
int src = 0, dest = 1, tag = 100, flag = 0, flag2 = 0;
int myrank, provided, len, i, j;
char hostname[MPI_MAX_PROCESSOR_NAME];
MPI_Count partitions = PARTITIONS;
MPI_Request request;

//Initializing threaded MPI
MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
if (provided < MPI_THREAD_SERIALIZED)
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
MPI_Get_processor_name(hostname, &len);

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
                MPI_Pready(i, request);
        }
	
	//Test for overall send operation completion
        while (!flag)
        {
                MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
        }
	
	//This Barrier allows task 1 to proceed
	MPI_Barrier(MPI_COMM_WORLD);

        MPI_Request_free(&request);
}
else if (myrank == 1)
{
	//This Barrier prevents the task 1 to run before the partitioned send completes
	MPI_Barrier(MPI_COMM_WORLD);

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

