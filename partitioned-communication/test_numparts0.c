// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 *
 * Test 4: P. 114, line 12-13: The number of user-visible partitions on the send and receiver side may differ. 
 * 
 * This test has twice as many send partitions as recv partitions
 *
 * Expected outcome: PASS
 *
 */

#include "test_common.h" 

#define PARTITIONS 8 
#define COUNT 8

int main(int argc, char *argv[]) {

    int message[PARTITIONS*COUNT];

    MPI_Count send_partitions = PARTITIONS * 2;
    MPI_Count send_count = (int)COUNT/2;
    MPI_Count recv_partitions = PARTITIONS;
    MPI_Count recv_count = COUNT;

    int source = 0, dest = 1, tag = 1, flag = 0; 
    int myrank, i, j;
    int provided;

    for (i = 0; i < PARTITIONS * COUNT; ++i) message[i] = 0;

    MPI_Request request;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided); 
    if (provided < MPI_THREAD_SERIALIZED) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE); MPI_Comm_rank(MPI_COMM_WORLD , &myrank);

    if (0 == myrank) {

        MPI_Psend_init(message, send_partitions, send_count, MPI_INT, dest, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request);
        MPI_Start(&request);

        /* Fill the sending partitions and mark as ready as each is filled */
        for (i = 0; i < send_partitions; ++i) {
            for (j = 0; j < send_count; ++j) message[j+(send_count*i)] = j+(send_count*i);
            MPI_Pready(i, request);
        }

        while (!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE); 
        }

        MPI_Request_free(&request);

    }
    else if (1 == myrank) {
        MPI_Precv_init(message, recv_partitions, recv_count, MPI_INT, source, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request);
        MPI_Start(&request); 
        while (!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE); 
        }

        MPI_Request_free(&request);

        /* all partitions received; check contents */
        for (i = 0; i < recv_partitions*recv_count; ++i) {
            if (message[i] != i) {
                fprintf(stderr, "ERROR: Contents received do not match contents sent (expected %d, found %d).\n",i,message[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize();

    return 0;
}

