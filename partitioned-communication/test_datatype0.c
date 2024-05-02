// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * The examples in chapter 4 of the specification use different combinations of 
 * sender and receiver contiguous datatypes that for testing purposes should be 
 * considered outside the use of OpenMP pragmas.
 *
 * This test uses the same datatype on sender and receiver, and the same 
 * number of partitions.
 *
 * Expected outcome: PASS
 *
 */

#include "test_common.h"

#define PARTITIONS 64 
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
    MPI_Datatype xfer_type;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided); 
    if (provided < MPI_THREAD_MULTIPLE) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE); 
    MPI_Comm_rank(MPI_COMM_WORLD , &myrank); 

    /* Sender and receiver both use the same transfer datatype */
    MPI_Type_contiguous(partlength, MPI_INT, &xfer_type); 
    MPI_Type_commit(&xfer_type);

    if (0 == myrank) {

        for (i = 0; i < partitions * partlength; ++i) message[i] = 100;

        MPI_Psend_init(message, partitions, count, xfer_type, dest, tag, MPI_COMM_WORLD, info, &request);
        MPI_Start(&request);

        for (i = 0; i < partitions; ++i) {
            for (j = 0; j < partlength; ++j) {
                message[j + (i * partlength)] = j + (i * partlength);
            }
            MPI_Pready(i, request); 
        }

        while(!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE); 
        }

        MPI_Request_free(&request); 

    } else if (1 == myrank) {
        for (i = 0; i < partitions * partlength; ++i) message[i] = 101;

        MPI_Precv_init(message, partitions, count, xfer_type, source, tag, MPI_COMM_WORLD, info, &request);
        MPI_Start(&request); 

        while(!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
        }

        MPI_Request_free(&request); 

        /* all partitions received; check contents */
        for (i = 0; i < MESSAGE_LENGTH; ++i) {
            if (message[i] != i) {
                fprintf(stderr, "ERROR: Contents received do not match contents sent (expected %d, found %d).\n",i,message[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (myrank == 0) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize ();
    return 0;

}
