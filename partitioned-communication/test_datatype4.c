// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

/* 
 * The examples in chapter 4 of the specification use different combinations of 
 * sender and receiver contiguous datatypes that for testing purposes should be 
 * considered outside the use of OpenMP pragmas.
 *
 * This test is simply a sanity check to make sure the combination of sender-side 
 * contiguous datatype and receiver-side non-derived datatype works with basic non-blocking send/recv.
 *
 * Expected outcome: PASS
 *
 */

#include "test_common.h"

#define MESSAGE_LENGTH 256
#define DATATYPE_LENGTH 128

int main(int argc, char *argv[]) {

    int message[MESSAGE_LENGTH];

    int count = 2, source = 0, dest = 1, tag = 1, flag = 0;
    int i, j;
    int myrank;
    int provided;
    int my_thread_id;

    MPI_Request request;

    MPI_Info info = MPI_INFO_NULL;
    MPI_Datatype send_type;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided); 

    if (provided < MPI_THREAD_MULTIPLE) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE); 

    MPI_Comm_rank(MPI_COMM_WORLD , &myrank); 

    /* Sender uses this datatype */
    MPI_Type_contiguous(DATATYPE_LENGTH, MPI_INT, &send_type); 
    MPI_Type_commit(&send_type);

    if (0 == myrank) {

        for (i = 0; i < MESSAGE_LENGTH; ++i) message[i] = i;

        MPI_Isend(message, count, send_type, dest, tag, MPI_COMM_WORLD, &request);

        MPI_Wait(&request, MPI_STATUS_IGNORE);

    } else if (1 == myrank) {

        for (i = 0; i < MESSAGE_LENGTH; ++i) message[i] = 101;

        MPI_Irecv(message, MESSAGE_LENGTH, MPI_INT, source, tag, MPI_COMM_WORLD, &request);

        MPI_Wait(&request, MPI_STATUS_IGNORE);

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

    MPI_Finalize ();
    return 0;

}
