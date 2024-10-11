/*
 * Copyright (c) 2024      Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Additional copyrights may follow
 *
 */

#include <iostream>
#include <chrono>
#include <random>

#include <mpi.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#define ERROR_CHECK( err, errlab ) if(err) { printf("ERROR: An error (%d) in an MPI call was detected at %s:%d!\n", err, __FILE__, __LINE__); goto errlab; }

int datatype_check() {
    int err, size;
    int block_count = 3, per_block = 3;
    MPI_Datatype dtype;
    MPI_Aint lb, extent, lb_true, extent_true;

    /* a contiguous block going backwards */
    err = MPI_Type_vector(block_count, per_block, -per_block, MPI_UINT8_T, &dtype);
    ERROR_CHECK( err, on_error );
    MPI_Type_commit(&dtype);

    err = MPI_Type_get_extent(dtype, &lb, &extent);
    ERROR_CHECK( err, on_error);
    err = MPI_Type_get_true_extent(dtype, &lb_true, &extent_true);
    ERROR_CHECK( err, on_error);
    err = MPI_Type_size(dtype, &size);
    ERROR_CHECK( err, on_error);
    printf("Datatype (normal,true) extents (%ld,%ld), size (%d,--), and lb (%ld,%ld)\n",
            extent, extent_true, size, lb, lb_true);

    if (0) {
on_error:
        MPI_Finalize();
        return 1;
    }
    return 0;
}
int check_bounds() {

    int err, size;
    const int nbytes = 25;
    uint8_t buf_bytes[nbytes];
    uint8_t buf_msg[nbytes];
    MPI_Aint lb, extent;

    for (int j=0; j<nbytes; j++) { buf_bytes[j] = j+1; };
    memset(buf_msg, 0, nbytes);

    int displ0 = 0;
    MPI_Aint array_of_displs[2] = {displ0, displ0+4};
    int array_of_block_lengths[2] = {1,1};
    MPI_Datatype types[2] = {MPI_CHAR, MPI_INT64_T};
    MPI_Datatype dtype;
    MPI_Type_create_struct(2, array_of_block_lengths, array_of_displs, types, &dtype);
    MPI_Type_commit(&dtype);
    MPI_Type_get_extent(dtype, &lb, &extent);
    MPI_Type_size(dtype, &size);
    printf("Extent: %ld.  LB=%ld\n", extent, lb);

    /* move data from pattern buf to message buf using send-to-self calls: */
    MPI_Request req;
    MPI_Irecv(buf_msg-lb+2, 1, dtype, 0, 0, MPI_COMM_SELF, &req);
    MPI_Send(buf_bytes,  size, MPI_BYTE, 0, 0, MPI_COMM_SELF);
    MPI_Wait(&req, MPI_STATUS_IGNORE);

    for (int j=0; j<nbytes; j++) {
        printf("%2ld ",j+lb-2);
    }
    printf(": Address\n");
    // for (int j=0; j<nbytes; j++) {
    //     printf("%2d ",buf_bytes[j]);
    // }
    // printf(": byte buffer\n");
    for (int j=0; j<nbytes; j++) {
        printf("%2d ",buf_msg[j]);
    }
    printf(": fill buffer\n");

    return 0;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    // datatype_check();
    check_bounds();
    MPI_Finalize();
}