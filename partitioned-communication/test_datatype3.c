// Copyright 2022 National Technology & Engineering Solutions of Sandia, LLC 
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
// Government retains certain rights in this software.

/* 
 * The examples in chapter 4 of the specification use different combinations of 
 * sender and receiver contiguous datatypes that for testing purposes should be 
 * considered outside the use of OpenMP pragmas.
 *
 * This test has the sender use a datatype multiple times (one per partition) and 
 * the receiver receive into a single partition with a single datatype.
 *
 * Compare with test_datatype5.c, which does the same thing with Isend/Irecv insteaad of partcomm
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
    // If the test seg faults, it may be a memory alignment issue
    //int *message = NULL;
    //if (posix_memalign((void *)&message, 32, sizeof(int) * MESSAGE_LENGTH)) {
    //  fprintf(stderr, "Error using posix_memalign\n");
    //  exit(-1);
    //}
    int send_partitions = PARTITIONS;
    int send_partlength = PARTLENGTH;
    int recv_partitions = 1;
    int recv_partlength = MESSAGE_LENGTH;

    int send_count = 1, recv_count = 1, source = 0, dest = 1, tag = 1, flag = 0;
    int i, j;
    int myrank;
    int provided;
    int my_thread_id;

    MPI_Request request;
    MPI_Status status;
    MPI_Info info = MPI_INFO_NULL;
    MPI_Datatype send_type, recv_type;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided); 
    if (provided < MPI_THREAD_MULTIPLE) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE); 
    MPI_Comm_rank(MPI_COMM_WORLD , &myrank); 

    /* sender uses PARTITIONS partitions each comprising one instance of a PARTLENTH contiguous datatype */
    MPI_Type_contiguous(send_partlength, MPI_INT, &send_type); 
    MPI_Type_commit(&send_type);

    /* receier uses 1 partition comprising one instance of a MESSAGE_LENGTH contiguous datatype */
    MPI_Type_contiguous(recv_partlength, MPI_INT, &recv_type);
    MPI_Type_commit(&recv_type);

    if (0 == myrank) {

        for (i = 0; i < send_partitions * send_partlength; ++i) message[i] = 100;

        MPI_Psend_init(message, send_partitions, send_count, send_type, dest, tag, MPI_COMM_WORLD, info, &request);
        MPI_Start(&request);

        for (i = 0; i < send_partitions; ++i) {
            for (j = 0; j < send_partlength; ++j) {
                message[j + (i * send_partlength)] = j + (i * send_partlength);
            }
            MPI_Pready(i, request); 
        }

        while(!flag) {
            MPI_Test(&request, &flag, MPI_STATUS_IGNORE); 
        }

        MPI_Request_free(&request);

    } else if (1 == myrank) {
        for (i = 0; i < send_partitions * send_partlength; ++i) message[i] = 101;

        MPI_Precv_init(message, recv_partitions, recv_count, recv_type, source, tag, MPI_COMM_WORLD, info, &request);
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
    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize();
    return 0;

}
