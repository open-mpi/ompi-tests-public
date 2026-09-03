/*Partitions MPI Unit Test
 *
 * Shows the behavior of the communication when a partitioned recv / send corridor is declared.
 * Parrived loops trying to find a recieved partition before any are sent and fails. 
 * Parrived should timeout.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "assert.h"
#include "time.h"

#define PARTITIONS 8
#define COUNT 5
#define TIMEOUT 10000

int main (int argc, char *argv[])
{
//Buffer message
double message [PARTITIONS * COUNT];

//MPI variables declarations
int src = 0, dest = 1, tag = 100, flag = 0, flag2 = 0;
int myrank, provided, timer, trigger, i, j;
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
        
	//This Barrier prevents task 0 to initialize a send operation before task 0 loops through Parrived 
	
	MPI_Barrier(MPI_COMM_WORLD);

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
        MPI_Request_free(&request);
}
else if (myrank == 1)
{
        MPI_Precv_init(message, partitions, COUNT, MPI_DOUBLE, src, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &request);
        MPI_Start(&request);
	
	//sets a timer in milliseconds equal to the time passed since beginning of operation
	timer = clock() * 1000 / CLOCKS_PER_SEC;
	//creates a trigger by adding a timeout time in millisecond to the current time passed
	trigger = timer + TIMEOUT;

	//Iterates through the partitions and check indefinetly to see if they have arrived	
	for (i = 0; i < partitions; i++)
        {
                MPI_Parrived(request, i, &flag2);
                if (!flag2) {
                        i--;
                }
                else {
			//Test the buffer to check that the message was recieved correctly
                        for (j = (i * COUNT); j < ((i+1) * COUNT); j++)
                        {
                                assert(message[j] == (j + 1));
                        }
                }
		//set timer equal to the current time elapsed
		timer = clock() * 1000 / CLOCKS_PER_SEC;
		//Abort MPI if Parrived loops more than time time allowed
		if (timer > trigger){
			printf("Parrived Timeout, No Partitions recieved in %d millisecond", TIMEOUT);
			MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		}
        }
	
	//This barrier allows task 0 to proceed
	MPI_Barrier(MPI_COMM_WORLD);

	//Test for overall recieve operation completion
        while (!flag)
        {
                MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
        }
	
	printf("Test Passed Succesfully\n");
        MPI_Request_free(&request);
}
MPI_Finalize();
return 0;
}

