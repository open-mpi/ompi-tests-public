// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * P. 113, line 15: Part comm operations are matched based on the order in which the local initialization calls are performed.  
 * P. 117, line 10: In the event that the communicator, tag, and source do not uniquely identify a message, the order 
 * in which partitioned communication initialization calls are made is the order in which they will eventually match.
 * P. 118, line 24: If communicator, tag, and source are not enough to match, order is used.
 *
 * Expected outcome: PASS
 *
 */

#include "test_common.h" 

#define PARTITIONS 8 
#define COUNT 5

int main(int argc, char *argv[]) {

    int message0[PARTITIONS*COUNT];
    int message1[PARTITIONS*COUNT];

    MPI_Count partitions = PARTITIONS;

    int source = 0, dest = 1, tag = 1, flag0 = 0, flag1 = 0;
    int myrank, i, j;
    int provided;

    for (i = 0; i < PARTITIONS * COUNT; ++i) message0[i] = 0;
    for (i = 0; i < PARTITIONS * COUNT; ++i) message1[i] = 0;

    MPI_Request request0, request1;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided); 
    if (provided < MPI_THREAD_SERIALIZED) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE); MPI_Comm_rank(MPI_COMM_WORLD , &myrank);

    if (0 == myrank) {

        /* init in order 0, 1 */
        MPI_Psend_init(message0, partitions, COUNT, MPI_INT, dest, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request0);
        MPI_Psend_init(message1, partitions, COUNT, MPI_INT, dest, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request1);

        /* start both */
        MPI_Start(&request0);
        MPI_Start(&request1);

        /* fill 0; should fill 1 on receiver */
        for (i = 0; i < partitions; ++i) {
            for (j = 0; j < COUNT; ++j) message0[j+(COUNT*i)] = 10*(j+(COUNT*i));
            MPI_Pready(i, request0);
        }

        while (!flag0) {
            MPI_Test(&request0, &flag0, MPI_STATUS_IGNORE); 
        }

        /* fill 1; should fill 0 on receiver */
        for (i = 0; i < partitions; ++i) {
            for (j = 0; j < COUNT; ++j) message1[j+(COUNT*i)] = j+(COUNT*i);
            MPI_Pready(i, request1);
        }

        while (!flag1) {
            MPI_Test(&request1, &flag1, MPI_STATUS_IGNORE); 
        }

        MPI_Request_free(&request0);
        MPI_Request_free(&request1);

    }
    else if (1 == myrank) {
        /* init in order 1, 0 */
        MPI_Precv_init(message1, partitions, COUNT, MPI_INT, source, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request1);
        MPI_Precv_init(message0, partitions, COUNT, MPI_INT, source, tag, MPI_COMM_WORLD , MPI_INFO_NULL , &request0);

        MPI_Start(&request0); 
        MPI_Start(&request1); 

        while (!flag0 && !flag1) {
            MPI_Test(&request0, &flag0, MPI_STATUS_IGNORE); 
            MPI_Test(&request1, &flag1, MPI_STATUS_IGNORE); 
        }

        MPI_Request_free(&request0);
        MPI_Request_free(&request1);

        /* all partitions received; check contents */
        //fprintf(stderr, "Message 0: ");
        for (i = 0; i < PARTITIONS*COUNT; ++i) {
            //fprintf(stderr, "%d, ", message0[i]);
            if (message0[i] != i) {
                fprintf(stderr, "ERROR: Contents received in message buffer 0 do not match contents sent (expected %d, found %d).\n",i,message0[i]);
                MPI_Abort(MPI_COMM_WORLD, 0);
            }
        }
        //fprintf(stderr, "\n");
        //fprintf(stderr, "Message 1: ");
        for (i = 0; i < PARTITIONS*COUNT; ++i) {
            //fprintf(stderr, "%d, ", message1[i]);
            if (message1[i] != i*10) {
                fprintf(stderr, "ERROR: Contents received in message buffer 1 do not match contents sent (expected %d, found %d).\n",i,message1[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
        //fprintf(stderr, "\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize();

    return 0;
}

