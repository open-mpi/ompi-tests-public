// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * STARTALL should start multiple channels, since it is a generalization of MPI_START. (Section 3.9, pp. 109-110). 
 *
 * This test simply confirms this.
 *
 * Expected outcome: PASS
 *
 */

#include "test_common.h" 

#define PARTITIONS 8 
#define COUNT 5
#define CHANNELS 3

int main(int argc, char *argv[]) {

    int messageA[PARTITIONS*COUNT];
    int messageB[PARTITIONS*COUNT];
    int messageC[PARTITIONS*COUNT];

    MPI_Count partitions = PARTITIONS;

    int source = 0, dest = 1, tagA = 1, tagB = 2, tagC = 3, flagA = 0, flagB = 0, flagC = 0, sflag = 0; 
    int myrank, i, j;
    int provided;

    for (i = 0; i < PARTITIONS * COUNT; ++i) {
        messageA[i] = 0;
        messageB[i] = 0;
        messageC[i] = 0;
    }

    MPI_Request requests[CHANNELS];
    MPI_Status status;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided); 
    if (provided < MPI_THREAD_SERIALIZED) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE);
    MPI_Comm_rank(MPI_COMM_WORLD , &myrank);

    if (0 == myrank) {

        CHECK_RETVAL(MPI_Psend_init(messageA, partitions, COUNT, MPI_INT, dest, tagA, MPI_COMM_WORLD, MPI_INFO_NULL, &requests[0]));
        CHECK_RETVAL(MPI_Psend_init(messageB, partitions, COUNT, MPI_INT, dest, tagB, MPI_COMM_WORLD, MPI_INFO_NULL, &requests[1]));
        CHECK_RETVAL(MPI_Psend_init(messageC, partitions, COUNT, MPI_INT, dest, tagC, MPI_COMM_WORLD, MPI_INFO_NULL, &requests[2]));

        sflag = 0;
        MPI_Request_get_status(requests[0], &sflag, &status);
        fprintf(stderr, "After send init: request 0: %d\n", sflag);
        sflag = 0;
        MPI_Request_get_status(requests[1], &sflag, &status);
        fprintf(stderr, "After send init: request 1: %d\n", sflag);
        sflag = 0;
        MPI_Request_get_status(requests[2], &sflag, &status);
        fprintf(stderr, "After send init: request 2: %d\n", sflag);

        CHECK_RETVAL(MPI_Startall(CHANNELS, requests));

        sflag = 0;
        MPI_Request_get_status(requests[0], &sflag, &status);
        fprintf(stderr, "After startall: request 0: %d\n", sflag);
        sflag = 0;
        MPI_Request_get_status(requests[1], &sflag, &status);
        fprintf(stderr, "After startall: request 1: %d\n", sflag);
        sflag = 0;
        MPI_Request_get_status(requests[2], &sflag, &status);
        fprintf(stderr, "After startall: request 2: %d\n", sflag);

        /* Fill the sending partitions and mark as ready as each is filled */
        for (i = 0; i < partitions; ++i) {
            for (j = 0; j < COUNT; ++j) messageA[j+(COUNT*i)] = j+(COUNT*i);
            CHECK_RETVAL(MPI_Pready(i, requests[0]));
            for (j = 0; j < COUNT; ++j) messageB[j+(COUNT*i)] = (j+(COUNT*i)) * 10;
            CHECK_RETVAL(MPI_Pready(i, requests[1]));
            for (j = 0; j < COUNT; ++j) messageC[j+(COUNT*i)] = (j+(COUNT*i)) * 100;
            CHECK_RETVAL(MPI_Pready(i, requests[2]));
        }

        sflag = 0;
        MPI_Request_get_status(requests[0], &sflag, &status);
        fprintf(stderr, "After pready: request 0: %d\n", sflag);
        sflag = 0;
        MPI_Request_get_status(requests[1], &sflag, &status);
        fprintf(stderr, "After pready: request 1: %d\n", sflag);
        sflag = 0;
        MPI_Request_get_status(requests[2], &sflag, &status);
        fprintf(stderr, "After pready: request 2: %d\n", sflag);


        fprintf(stderr, "Rank 0: testing\n");

        while ( !(flagA && flagB && flagC) )  {
            CHECK_RETVAL(MPI_Test(&requests[0], &flagA, MPI_STATUS_IGNORE)); 
            CHECK_RETVAL(MPI_Test(&requests[1], &flagB, MPI_STATUS_IGNORE)); 
            CHECK_RETVAL(MPI_Test(&requests[2], &flagC, MPI_STATUS_IGNORE)); 
        }

        fprintf(stderr, "Rank 1: all flags True\n");

        MPI_Request_free(&requests[0]);
        MPI_Request_free(&requests[1]);
        MPI_Request_free(&requests[2]);

    }
    else if (1 == myrank) {
        CHECK_RETVAL(MPI_Precv_init(messageA, partitions, COUNT, MPI_INT, source, tagA, MPI_COMM_WORLD, MPI_INFO_NULL, &requests[0]));
        CHECK_RETVAL(MPI_Precv_init(messageB, partitions, COUNT, MPI_INT, source, tagB, MPI_COMM_WORLD, MPI_INFO_NULL, &requests[1]));
        CHECK_RETVAL(MPI_Precv_init(messageC, partitions, COUNT, MPI_INT, source, tagC, MPI_COMM_WORLD, MPI_INFO_NULL, &requests[2]));

        CHECK_RETVAL(MPI_Startall(CHANNELS, requests)); 

        fprintf(stderr, "Rank 1: testing\n");
        while ( !(flagA && flagB && flagC) ) {
            CHECK_RETVAL(MPI_Test(&requests[0], &flagA, MPI_STATUS_IGNORE));
            CHECK_RETVAL(MPI_Test(&requests[1], &flagB, MPI_STATUS_IGNORE));
            CHECK_RETVAL(MPI_Test(&requests[2], &flagC, MPI_STATUS_IGNORE));
        }

        MPI_Request_free(&requests[0]);
        MPI_Request_free(&requests[1]);
        MPI_Request_free(&requests[2]);

        fprintf(stderr, "Message A: ");
        for (i=0; i< PARTITIONS*COUNT; ++i) fprintf(stderr, "%d ", messageA[i]);
        fprintf(stderr, "\n");
        fprintf(stderr, "Message B: ");
        for (i=0; i< PARTITIONS*COUNT; ++i) fprintf(stderr, "%d ", messageB[i]);
        fprintf(stderr, "\n");
        fprintf(stderr, "Message C: ");
        for (i=0; i< PARTITIONS*COUNT; ++i) fprintf(stderr, "%d ", messageC[i]);
        fprintf(stderr, "\n");

        /* all partitions received; check contents */
        for (i = 0; i < PARTITIONS*COUNT; ++i) {
            if (messageA[i] != i) {
                fprintf(stderr, "ERROR: Contents of buffer messageA do not match contents sent (expected %d, found %d).\n",i,messageA[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            if (messageB[i] != i*10) {
                fprintf(stderr, "ERROR: Contents of buffer messageB do not match contents sent (expected %d, found %d).\n",i*10,messageB[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            if (messageC[i] != i*100) {
                fprintf(stderr, "ERROR: Contents of buffer messageC do not match contents sent (expected %d, found %d).\n",i*100,messageC[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize ();

    return 0;
}

