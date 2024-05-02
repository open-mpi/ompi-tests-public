// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * This is a variation on example 4.2, p. 122-123, MPI Standard v. 4.1
 *
 * Note that this example (like the original) uses the same congituous datatype on both 
 * sender and receiver. See test_example3x.c for other variations.
 *
 * Expected outcome: PASS
 *
 */

#include <omp.h>
#include "test_common.h"

#define NUM_THREADS 8
#define PARTITIONS 8
#define PARTLENGTH 16

int main(int argc, char *argv[]) {

    int message[PARTITIONS*PARTLENGTH];
    int partitions = PARTITIONS;
    int partlength = PARTLENGTH;
    int count = 1, source = 0, dest = 1, tag = 1, flag = 0; 
    int i, j;
    int my_thread_id;
    int myrank;
    int provided;

    omp_set_dynamic(0); // Disable dynamic teams
    omp_set_num_threads(NUM_THREADS);

    for (i = 0; i < partitions * partlength; ++i) message[i] = 0;

    MPI_Request request;
    MPI_Info info = MPI_INFO_NULL;
    MPI_Datatype xfer_type;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided); 
    if (provided < MPI_THREAD_MULTIPLE) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE);

    MPI_Comm_rank(MPI_COMM_WORLD , &myrank); 
    MPI_Type_contiguous(partlength , MPI_INT, &xfer_type); 
    MPI_Type_commit(&xfer_type);

    if (0 == myrank) {

        MPI_Psend_init(message, partitions, count, xfer_type, dest, tag, MPI_COMM_WORLD, info, &request); 
        MPI_Start(&request);

#pragma omp parallel for shared(request,message) private(j, my_thread_id) num_threads(NUM_THREADS)
        for (i=0; i<partitions; i++) {
            my_thread_id = omp_get_thread_num();
            for (j = 0; j < partlength; ++j) {
                message[j+(partlength*my_thread_id)] = j+(partlength*my_thread_id);
            }
            MPI_Pready(i, request); 
        }

        //    fprintf(stderr, "Sent: ");
        //    for (i = 0; i < partlength * partitions; ++i) fprintf(stderr, "%d ", message[i]);
        //    fprintf(stderr, "\n");

        while(!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
        }
        MPI_Request_free(&request); 

        MPI_Barrier(MPI_COMM_WORLD);
    }
    else if (1 == myrank) {

        MPI_Precv_init(message, partitions, count, xfer_type, source, tag, MPI_COMM_WORLD, info, &request);
        MPI_Start(&request); 
        while(!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE); 
        }
        MPI_Request_free(&request); 

        //    fprintf(stderr, "Received: ");
        //    for (i = 0; i < partlength * partitions; ++i) fprintf(stderr, "%d ", message[i]);
        //    fprintf(stderr, "\n");

        /* all partitions received; check contents */
        for (i = 0; i < partitions*partlength; ++i) {
            if (message[i] != i) {
                fprintf(stderr, "ERROR: Contents received do not match contents sent (expected %d, found %d).\n",i,message[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize ();
    return 0;
}
