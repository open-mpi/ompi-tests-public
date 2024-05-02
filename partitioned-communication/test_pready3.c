// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * Calls pready on a partition that is active, i.e., has already been marked as ready.
 *
 * P. 119, line 11-12: Calling MPI_PREADY on an active partition is erroneous. 
 *
 * Expected outcome: Error message
 */

#include "test_common.h"

#define PARTITIONS 8 
#define COUNT 5

int main(int argc, char *argv[]) {

    int message[PARTITIONS*COUNT];

    MPI_Count partitions = PARTITIONS;
    MPI_Count active_partition = 2; // assumes at least 3 partitions

    int source = 0, dest = 1, tag = 1, flag = 0; 
    int myrank, i, j;
    int provided;

    for (i = 0; i < PARTITIONS * COUNT; ++i) message[i] = 0;

    MPI_Request request;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided); 
    if (provided < MPI_THREAD_SERIALIZED) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE); MPI_Comm_rank(MPI_COMM_WORLD , &myrank);

    if (0 == myrank) {

        CHECK_RETVAL(MPI_Psend_init(message, partitions, COUNT, MPI_INT, dest, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request));
        CHECK_RETVAL(MPI_Start(&request));

        for (i = 0; i < partitions; ++i) {
            for (j = 0; j < COUNT; ++j) message[j+(COUNT*i)] = j+(COUNT*i);
            CHECK_RETVAL(MPI_Pready(i, request));
        }

        /* Call Pready on a partition already marked as ready; an error should occur */
        fprintf(stderr, "Marking previously marked partition ready...\n");
        CHECK_RETVAL(MPI_Pready(active_partition, request));
        fprintf(stderr, "Previously marked partition marked again...\n");

        while (!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE); 
        }

        MPI_Request_free(&request);

    }
    else if (1 == myrank) {
        MPI_Precv_init(message, partitions, COUNT, MPI_INT, source, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request);
        MPI_Start(&request); 
        while (!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE); 
        }

        MPI_Request_free(&request);

        /* all partitions received; check contents */
        for (i = 0; i < PARTITIONS*COUNT; ++i) {
            if (message[i] != i) {
                fprintf(stderr, "ERROR: Contents received do not match contents sent (expected %d, found %d).\n",i,message[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize ();

    return 0;
}

