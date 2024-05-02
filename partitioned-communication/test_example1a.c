// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * This is example 4.1, p. 114-115, MPI Standard v. 4.1
 * Extended to (1) fill buffer, (2) test that correct results are received.
 *
 * Expected outcome: PASS
 *
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
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided); 
    if (provided < MPI_THREAD_SERIALIZED) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE);
    MPI_Comm_rank(MPI_COMM_WORLD , &myrank);

    if (0 == myrank) {

        CHECK_RETVAL(MPI_Psend_init(message, partitions, COUNT, MPI_INT, dest, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request));
        CHECK_RETVAL(MPI_Start(&request));

        /* Fill the sending partitions and mark as ready as each is filled */
        for (i = 0; i < partitions; ++i) {
            for (j = 0; j < COUNT; ++j) message[j+(COUNT*i)] = j+(COUNT*i);
            CHECK_RETVAL(MPI_Pready(i, request));
        }

        while (!flag) {
            CHECK_RETVAL(MPI_Test(&request, &flag, MPI_STATUS_IGNORE)); 
        }

        MPI_Request_free(&request);

    }
    else if (1 == myrank) {
        CHECK_RETVAL(MPI_Precv_init(message, partitions, COUNT, MPI_INT, source, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request));
        CHECK_RETVAL(MPI_Start(&request)); 
        while (!flag) {
            CHECK_RETVAL(MPI_Test(&request, &flag, MPI_STATUS_IGNORE));
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

