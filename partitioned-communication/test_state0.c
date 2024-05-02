// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * This test is motivated by https://github.com/open-mpi/ompi/issues/12328
 *
 * The test confirms that the request status is reset in subsequent rounds using the same Send/Recv channel.
 * 
 * On the sender side, if the request status does not reset with the second call to START, marking partitions 
 * with PREADY will do nothing, and the TEST/WAIT will automatically succeed with no data having been sent.
 *
 * On the reciever side, if the request status does not reset with the second call to START, the receiver will 
 * immediately succeed on TEST/WAIT and then the test will fail when validating the buffer contents (because the 
 * receiver did not get round 2 data). If the request status is reset, then the test will hang waiting for 
 * completion.
 *
 * The test is passed if it runs to completion successfully.
 *
 */

#include "test_common.h" 

#define PARTITIONS 4 
#define COUNT 5

int main(int argc, char *argv[]) {

    int message[PARTITIONS*COUNT];

    MPI_Count partitions = PARTITIONS;

    int source = 0, dest = 1, tag = 1, flag = 0; 
    int myrank, i, j;
    int provided;

    for (i = 0; i < PARTITIONS * COUNT; ++i) message[i] = 0;

    MPI_Request request;
    MPI_Status status;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided); 
    if (provided < MPI_THREAD_SERIALIZED) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE); MPI_Comm_rank(MPI_COMM_WORLD , &myrank);

    if (0 == myrank) {

        MPI_Psend_init(message, partitions, COUNT, MPI_INT, dest, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request);

        flag = 0;
        MPI_Request_get_status(request, &flag, &status);
        fprintf(stderr, "Rank 0: Communication Round 1: After PSEND_INIT, request status is: %d\n", flag);

        /* Round 1 */
        MPI_Start(&request);

        flag = 0;
        MPI_Request_get_status(request, &flag, &status);
        fprintf(stderr, "Rank 0: Communication Round 1:  After START, request status is: %d\n", flag);

        for (i = 0; i < partitions; ++i) {
            for (j = 0; j < COUNT; ++j) message[j+(COUNT*i)] = j+(COUNT*i);
            MPI_Pready(i, request);
            flag = 0;
            MPI_Request_get_status(request, &flag, &status);
            fprintf(stderr, "Rank 0: Communication Round 1:  After PREADY #%d, request status is: %d\n", i, flag);
        }

        fprintf(stderr, "Sent in round 1: ");
        for (i=0; i < COUNT * PARTITIONS; ++i) fprintf(stderr, " %d", message[i]);
        fprintf(stderr, "\n");

        flag = 0;
        while (!flag) {
            CHECK_RETVAL(MPI_Test(&request, &flag, MPI_STATUS_IGNORE)); 
        }
        flag = 0;
        MPI_Request_get_status(request, &flag, &status);
        fprintf(stderr, "Rank 0: Communication Round 1:  After TEST, request status is: %d\n", flag);

        /* Round 2 */
        MPI_Start(&request);

        flag = 0;
        MPI_Request_get_status(request, &flag, &status);
        fprintf(stderr, "Rank 0: Communication Round 2:  After START, request status is: %d\n", flag);

        /* Fill the sending partitions and mark as ready as each is filled; note data is different than first round */
        for (i = 0; i < partitions; ++i) {
            for (j = 0; j < COUNT; ++j) message[j+(COUNT*i)] = 10*(j+(COUNT*i));
            MPI_Pready(i, request);
            flag = 0;
            MPI_Request_get_status(request, &flag, &status);
            fprintf(stderr, "Rank 0: Communication Round 2:  After PREADY #%d, request status is: %d\n", i, flag);
        }

        fprintf(stderr, "Sent in round 2: ");
        for (i=0; i < COUNT * PARTITIONS; ++i) fprintf(stderr, " %d", message[i]);
        fprintf(stderr, "\n");

        flag = 0;
        while (!flag) {
            CHECK_RETVAL(MPI_Test(&request, &flag, MPI_STATUS_IGNORE)); 
        }
        flag = 0;
        MPI_Request_get_status(request, &flag, &status);
        fprintf(stderr, "Rank 0: Communication Round 2:  After TEST, request status is: %d\n", flag);

        MPI_Request_free(&request);

    }
    else if (1 == myrank) {
        /* Round 1 */
        CHECK_RETVAL(MPI_Precv_init(message, partitions, COUNT, MPI_INT, source, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request));
        CHECK_RETVAL(MPI_Start(&request)); 
        while (!flag) {
            CHECK_RETVAL(MPI_Test(&request, &flag, MPI_STATUS_IGNORE));
        }

        fprintf(stderr, "Received in round 1: ");
        for (i=0; i < COUNT * PARTITIONS; ++i) fprintf(stderr, " %d", message[i]);
        fprintf(stderr, "\n");

        /* Round 2 */
        CHECK_RETVAL(MPI_Start(&request)); 
        while (!flag) {
            CHECK_RETVAL(MPI_Test(&request, &flag, MPI_STATUS_IGNORE));
        }

        MPI_Request_free(&request);

        fprintf(stderr, "Received in round 2: ");
        for (i=0; i < COUNT * PARTITIONS; ++i) fprintf(stderr, " %d", message[i]);
        fprintf(stderr, "\n");

        /* all partitions received; check contents */
        for (i = 0; i < PARTITIONS*COUNT; ++i) {
            if (message[i] != 10*i) {
                fprintf(stderr, "ERROR: Contents received do not match contents sent (expected %d, found %d).\n",10*i,message[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize ();

    return 0;
}

