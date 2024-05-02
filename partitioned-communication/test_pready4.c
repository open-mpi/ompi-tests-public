// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * Calls PREADY on a request object that corresponds to an Isend.
 *
 * P. 119, line 6: "It is erroneous to use MPI_PREADY on any request object that does not correspond to a partitioned send operation."
 *
 * Expected outcome: Error message
 */

#include "test_common.h"

#define PARTITIONS 8 
#define COUNT 5

int main(int argc, char *argv[]) {

    int message[PARTITIONS*COUNT];

    MPI_Count partitions = PARTITIONS;

    int source = 0, dest = 1, tag = 1, flag = 0; 
    int myrank, i, j;
    int provided;

    for (i = 0; i < PARTITIONS * COUNT; ++i) message[i] = 0;

    MPI_Request request;
    MPI_Request isend_request;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided); 
    if (provided < MPI_THREAD_SERIALIZED) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE); MPI_Comm_rank(MPI_COMM_WORLD , &myrank);

    if (0 == myrank) {

        CHECK_RETVAL(MPI_Isend(message, PARTITIONS*COUNT, MPI_INT, dest, tag, MPI_COMM_WORLD, &isend_request));


        // This call is erroneous
        CHECK_RETVAL(MPI_Pready(0, isend_request));

        CHECK_RETVAL(MPI_Wait(&isend_request, MPI_STATUS_IGNORE));

    }
    else if (1 == myrank) {

        MPI_Irecv(message, PARTITIONS*COUNT, MPI_INT, source, tag, MPI_COMM_WORLD, &request);

        MPI_Wait(&request, MPI_STATUS_IGNORE);

    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize ();

    return 0;
}

