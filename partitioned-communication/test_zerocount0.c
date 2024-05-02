// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * In PSEND_INIT and PRECV_INIT, partitions and count can be zero.
 * P. 116, line 19: In PSEND_INIT, partitions is defined as “number of partitions (non-negative integer)”
 * P. 116, line 20: In PSEND_INIT, count is defined as “number of elements per partition (non-negative integer)”. 
 * Same for PRECV_INIT (p. 117, line 27 and p. 117, line 29)
 *
 * This test confirms nothing goes wrong when both PSEND_INIT and PRECV_INIT use 0 partitions with a > 0 count.
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

    MPI_Count comm_partitions = 0;  // use 0 partitions for init
    MPI_Count comm_count = COUNT;   // use COUNT partitions for init

    int source = 0, dest = 1, tag = 1, flag = 0; 
    int myrank, i, j;
    int provided;

    for (i = 0; i < PARTITIONS * COUNT; ++i) message[i] = 0;

    MPI_Request request;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided); 
    if (provided < MPI_THREAD_SERIALIZED) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE);
    MPI_Comm_rank(MPI_COMM_WORLD , &myrank);

    if (0 == myrank) {

        CHECK_RETVAL(MPI_Psend_init(message, comm_partitions, comm_count, MPI_INT, dest, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request));
        CHECK_RETVAL(MPI_Start(&request));

        /* Fill the sending partitions and mark as ready as each is filled */
        /* Because there are 0 partitions, Pready never gets called */
        for (i = 0; i < comm_partitions; ++i) {
            for (j = 0; j < COUNT; ++j) message[j+(COUNT*i)] = j+(COUNT*i);
            CHECK_RETVAL(MPI_Pready(i, request));
        }

        while (!flag) {
            CHECK_RETVAL(MPI_Test(&request, &flag, MPI_STATUS_IGNORE)); 
        }

        MPI_Request_free(&request);

    }
    else if (1 == myrank) {
        CHECK_RETVAL(MPI_Precv_init(message, comm_partitions, comm_count, MPI_INT, source, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request));
        CHECK_RETVAL(MPI_Start(&request)); 
        while (!flag) {
            CHECK_RETVAL(MPI_Test(&request, &flag, MPI_STATUS_IGNORE));
        }

        MPI_Request_free(&request);

    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize ();

    return 0;
}

