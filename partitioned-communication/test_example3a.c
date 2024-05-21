// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * Variation on Example 4.3, pp. 123-124
 *
 * Original example:
 *  - Uses MPI_DOUBLE
 *  - Uses derived contiguous datatype on send side and MPI_DOUBLE on recv side
 *  - Has multiple partitions on send side and a single partition on recv side
 *
 * This version:
 *  - Uses MPI_INT
 *  - Does not use contiguous datatype on either side
 *  - Has an equal number of partitions on send and recv side
 *
 * Expected outcome: PASS
 *
 */

#include <omp.h>
#include "test_common.h"

#define NUM_THREADS 8
#define NUM_TASKS 64
#define PARTITIONS NUM_TASKS
#define PARTLENGTH 16
#define MESSAGE_LENGTH PARTITIONS*PARTLENGTH

int main(int argc, char *argv[]) {

    int message[MESSAGE_LENGTH];
    int partitions = PARTITIONS;
    int partlength = PARTLENGTH;

    int count = 1, source = 0, dest = 1, tag = 1, flag = 0;
    int i, j;
    int myrank;
    int provided;
    int my_thread_id;

    MPI_Request request;
    MPI_Status status;
    MPI_Info info = MPI_INFO_NULL;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided); 
    if (provided < MPI_THREAD_MULTIPLE) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE); 
    MPI_Comm_rank(MPI_COMM_WORLD , &myrank); 

    if (0 == myrank) {

        for (i = 0; i < partitions * partlength; ++i) message[i] = 100;

        MPI_Psend_init(message, partitions, partlength, MPI_INT, dest, tag, MPI_COMM_WORLD, info, &request);
        MPI_Start(&request);

#pragma omp parallel shared(request,message) private(i, my_thread_id) num_threads(NUM_THREADS) 
        {
#pragma omp single 
            {
                /* single thread creates 64 tasks to be executed by 8 threads */ 
                for (int task_num=0; task_num < NUM_TASKS; task_num++) {
#pragma omp task firstprivate(task_num)
                    {
                        my_thread_id = omp_get_thread_num();
                        for (i=0; i < partlength; ++i) {
                            message[i + (task_num * partlength)] = i + (task_num * partlength);
                        }
                        MPI_Pready(task_num, request); 
                    } /* end task */
                } /* end for */ 
            } /* end single */
        } /* end parallel */ 

        //    fprintf(stderr, "Sent: ");
        //    for (i = 0; i < MESSAGE_LENGTH; ++i) fprintf(stderr, "%d ", message[i]);
        //    fprintf(stderr, "\n");

        while(!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE); 
        }

        MPI_Request_free(&request);

    } else if (1 == myrank) {
        for (i = 0; i < partitions * partlength; ++i) message[i] = 101;

        MPI_Precv_init(message, partitions, partlength, MPI_INT, source, tag, MPI_COMM_WORLD, info, &request);
        MPI_Start(&request); 

        while(!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
        }

        MPI_Request_free(&request); 

        //    fprintf(stderr, "Received: ");
        //    for (i = 0; i < MESSAGE_LENGTH; ++i) fprintf(stderr, "%d ", message[i]);
        //    fprintf(stderr, "\n");

        /* all partitions received; check contents */
        for (i = 0; i < MESSAGE_LENGTH; ++i) {
            if (message[i] != i) {
                fprintf(stderr, "ERROR: Contents received do not match contents sent (expected %d, found %d).\n",i,message[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize();
    return 0;

}
