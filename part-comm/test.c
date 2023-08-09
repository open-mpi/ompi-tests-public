#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#define PARTITIONS 8
#define COUNT 4

int main (int argc, char *argv[])
{
double message [PARTITIONS * COUNT];
int src = 0, dest = 1, tag = 100, flag = 0, flag2 = 0;
int send_partitions = PARTITIONS,
    send_partlength = COUNT,
    recv_partitions = PARTITIONS/2,
    recv_partlength = COUNT*2;		
int myrank, provided, i, j;
MPI_Request request;

MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
if (provided < MPI_THREAD_SERIALIZED)
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

if (myrank == 0)
{
        MPI_Psend_init(message, send_partitions, send_partlength, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &request);
        MPI_Start(&request);

        for (i = 0; i < send_partitions; i++)
        {
               	printf("Partition %d Ready to Send, Send Buff: ", i);
		for (j = (i*send_partlength); j < ((i+1)*send_partlength); j++)
                {
                        message[j] = j+1;
			printf(" %.2lf, ", message[j]);
                }
		printf("\n");
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
        MPI_Precv_init(message, recv_partitions, recv_partlength, MPI_DOUBLE, src, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &request);
        MPI_Start(&request);

        for (i = 0; i < recv_partitions; i++)
        {
                MPI_Parrived(request, i, &flag2);
                if (!flag2) {
                        i--;	
                }
                else {
                        printf("Partition %d Recieved Buff: ", i);
			for (j = (i*recv_partlength); j < ((i+1)*recv_partlength); j++)
                	{
                        	printf(" %.2lf, ", message[j]);
                	}
               		printf("\n");
		}
        }
        while (!flag)
        {
                MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
        }
        MPI_Request_free(&request);
}
MPI_Finalize();
return 0;
}
