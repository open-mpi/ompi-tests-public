/*Partitions MPI Unit Test
 *
 * Shows the behavior of partitioned communication when there is different partitions size 
 * from send-side and recv-side.
 * Buffer size stays the same.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "assert.h"

#define PARTITIONS 5
#define COUNT 3

int main (int argc, char *argv[])
{
//Buffer message
double message [PARTITIONS * COUNT];

//Sets send side partitions to the provided constant and recv-side partitions to half 
//(int floors to the nearest lowest number in case of decimal)
int send_partitions = PARTITIONS,
    send_partlength = COUNT,
    recv_partitions = PARTITIONS/2,
    recv_partlength = COUNT*2;		

//MPI variables declarations
int src = 0, dest = 1, tag = 100, flag = 0;
int myrank, provided, i, j;
MPI_Request request;

//Initializing threaded MPI
MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
if (provided < MPI_THREAD_SERIALIZED)
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

if (myrank == 0)
{
        MPI_Psend_init(message, send_partitions, send_partlength, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &request);
        MPI_Start(&request);

	//Iterating through each buffer partition, filling them and marking them ready to send
        for (i = 0; i < send_partitions; i++)
        {
		for (j = (i*send_partlength); j < ((i+1)*send_partlength); j++)
                {
                        message[j]= j + 1;
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
        MPI_Precv_init(message, recv_partitions, recv_partlength, MPI_DOUBLE, src, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &request);
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
        printf("Test Passed Succesful\n");
        MPI_Request_free(&request);
}
MPI_Finalize();
return 0;
}
