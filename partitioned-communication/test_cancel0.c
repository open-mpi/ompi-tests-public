// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * P. 116, line 11: Freeing or canceling a partitioned communication request that is active (i.e., initialized and started) and not completed is erroneous. 
 *
 * Expected outcome: Error message due to calling MPI_Cancel on a request that is active
 * If test times out, then the error was not caught by the MPI library
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

        fprintf(stderr, "Rank 0 initializing and starting Psend\n");
        CHECK_RETVAL(MPI_Psend_init(message, partitions, COUNT, MPI_INT, dest, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request));
        MPI_Start(&request);

        /* This should cause an error. See 4.1 p. 102 for info on cancel */
        fprintf(stderr, "Rank 0 canceling request (erroneous)\n");
        CHECK_RETVAL(MPI_Cancel(&request));

        /* Cancel is followed by wait, which should be local. If cancel ignored, then may hang here because nothing has been marked ready */
        fprintf(stderr, "Rank 0 waiting on cancelled request\n");
        CHECK_RETVAL(MPI_Wait(&request, MPI_STATUS_IGNORE));

        /* Now we the request is inactive and we can use it again or free it */
        MPI_Request_free(&request);

        /* Rest of code is the standard Psend/Precv example. If the MPI_Request_free is ignored and no error reported, 
         * then the remainder of the code may hang because the first Psend is never completed, and the destination is testing on it
         */

        fprintf(stderr, "Rank 0 starting second Psend\n");
        CHECK_RETVAL(MPI_Psend_init(message, partitions, COUNT, MPI_INT, dest, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request));
        CHECK_RETVAL(MPI_Start(&request));

        /* Fill the sending partitions and mark as ready as each is filled */
        for (i = 0; i < partitions; ++i) {
            for (j = 0; j < COUNT; ++j) message[j+(COUNT*i)] = j+(COUNT*i);
            CHECK_RETVAL(MPI_Pready(i, request));
        }

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

