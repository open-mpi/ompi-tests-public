// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 *
 * P. 117, line 13: PSEND_INIT can only match with partitioned communication initialization operations, therefore it is required to be matched with a corresponding MPI_PRECV_INIT.
 *
 * This test tries to match a PSEND_INIT with an IRECV. Because the PSEND_INIT should not match the 
 * IRECV, the code should timeout waiting
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

        MPI_Psend_init(message, partitions, COUNT, MPI_INT, dest, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request);
        MPI_Start(&request);

        /* Fill the sending partitions and mark as ready as each is filled */
        for (i = 0; i < partitions; ++i) {
            for (j = 0; j < COUNT; ++j) message[j+(COUNT*i)] = j+(COUNT*i);
            MPI_Pready(i, request);
        }

        fprintf(stderr, "Sending message: \n");
        for (i=0; i < partitions * COUNT; ++i) {
            fprintf(stderr, "%d, ", message[i]);
        }
        fprintf(stderr, "\n");

        while (!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE); 
        }

        MPI_Request_free(&request);

    }
    else if (1 == myrank) {

        fprintf(stderr, "Received message: \n");
        for (i=0; i < partitions * COUNT; ++i) {
            fprintf(stderr, "%d, ", message[i]);
        }
        fprintf(stderr, "\n");

        /* This should not work. Test should time out */
        fprintf(stderr, "Rank 1 posting Irecv\n");
        CHECK_RETVAL(MPI_Irecv(message, COUNT, MPI_INT, source, tag, MPI_COMM_WORLD, &request));
        fprintf(stderr, "Rank 1 waiting\n");
        CHECK_RETVAL(MPI_Wait(&request, MPI_STATUS_IGNORE)); /* should time out here */

        fprintf(stderr, "Received message: \n");
        for (i=0; i < partitions * COUNT; ++i) {
            fprintf(stderr, "%d, ", message[i]);
        }
        fprintf(stderr, "\n");

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

