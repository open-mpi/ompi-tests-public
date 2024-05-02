// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * P. 121, line 10: PARRIVED can be called on a null or inactive request; in this case, flag = true
 *
 * Expected outcome: PASS
 *
 */

#include <unistd.h>
#include "test_common.h" 

#define PARTITIONS 8 
#define COUNT 5

int main(int argc, char *argv[]) {

    int message[PARTITIONS*COUNT];

    MPI_Count partitions = PARTITIONS;
    int part_of_interest = 3;

    int source = 0, dest = 1, tag = 1, flag = 0; 
    int myrank, i, j;
    int provided;

    for (i = 0; i < PARTITIONS * COUNT; ++i) message[i] = 0;

    MPI_Request request;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided); 
    if (provided < MPI_THREAD_SERIALIZED) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE); MPI_Comm_rank(MPI_COMM_WORLD , &myrank);

    if (0 == myrank) {

        sleep(10);

    }
    else if (1 == myrank) {

        /* request never used; confirm parrived sets flag to true */
        if (!flag) fprintf(stderr, "Flag is false\n");
        while (!flag) {
            CHECK_RETVAL(MPI_Parrived(request, 0, &flag));
        }
        fprintf(stderr, "Flag is true\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize ();

    return 0;
}

