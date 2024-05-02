// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 *
 * P. 118, line 20: PRECV_INIT can only match with partitioned communication initialization operations, therefore 
 *    the MPI library is required to match MPI_PRECV_INIT calls only with a corresponding MPI_PSEND_INIT call.
 *
 * This version use persistent send
 *
 * Expected outcome: TIMEOUT
 *
 */

#include "test_common.h" 

#define PARTITIONS 1 
#define COUNT 64

int main(int argc, char *argv[]) {

    int message[PARTITIONS*COUNT];

    MPI_Count partitions = PARTITIONS;

    int source = 0, dest = 1, tag = 1, flag = 0; 
    int myrank, i, j;
    int provided;

    for (i = 0; i < PARTITIONS * COUNT; ++i) message[i] = 0;

    MPI_Request request;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided); 
    if (provided < MPI_THREAD_SERIALIZED) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE); MPI_Comm_rank(MPI_COMM_WORLD , &myrank);

    if (0 == myrank) {

        fprintf(stderr, "Rank 0 initializing persistent send\n");
        MPI_Send_init(message, COUNT, MPI_INT, dest, tag, MPI_COMM_WORLD, &request);

        /* Fill message to be sent */
        for (i = 0; i < partitions; ++i) {
            for (j = 0; j < COUNT; ++j) message[j+(COUNT*i)] = j+(COUNT*i);
        }

        fprintf(stderr, "Rank 0 starting persistent send\n");
        MPI_Start(&request);

        fprintf(stderr, "Rank 0 waiting\n");
        MPI_Wait(&request, MPI_STATUS_IGNORE); 

        fprintf(stderr, "Rank 0 done\n");

    }
    else if (1 == myrank) {

        fprintf(stderr, "Rank 1 initializing and starting Precv\n");
        MPI_Precv_init(message, partitions, COUNT, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_INFO_NULL, &request);
        MPI_Start(&request); 

        fprintf(stderr, "Rank 1 testing for completion ... should timeout here if Precv did not match persistent send\n");
        while (!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE); 
        }

        MPI_Request_free(&request);

        fprintf(stderr, "Rank 1 received message\n");

        /* all partitions received; check contents */
        for (i = 0; i < PARTITIONS*COUNT; ++i) {
            if (message[i] != i) {
                fprintf(stderr, "ERROR: Contents received do not match contents sent (expected %d, found %d).\n",i,message[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
        fprintf(stderr, "Rank 1: Contents verfied\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize ();

    return 0;
}

