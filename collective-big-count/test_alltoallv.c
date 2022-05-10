/*
 * Copyright (c) 2022 IBM Corporation.  All rights reserved.
 *
 * $COPYRIGHT$
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mpi.h>
#include "common.h"

int my_c_test_core(MPI_Datatype dtype, size_t total_num_elements, bool blocking);

int main(int argc, char** argv) {
    // Initialize the MPI environment
    int ret = 0;

    MPI_Init(NULL, NULL);
    init_environment(argc, argv);

    // Run the tests
#ifndef TEST_UNIFORM_COUNT
    // Buffer size: 2 GB
    // V_SIZE_INT tells us how many elements are needed to reach 2GB payload
    // Each rank will send/recv a count of V_SIZE_INT / world_size
    // The function will try to get as close to that as possible.
    //
    // Each rank contribues: V_SIZE_INT / world_size elements
    // Largest buffer is   : V_SIZE_INT elements
    ret += my_c_test_core(MPI_INT, V_SIZE_INT, true);
    ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, V_SIZE_DOUBLE_COMPLEX, true);
    if (allow_nonblocked) {
        ret += my_c_test_core(MPI_INT, V_SIZE_INT, false);
        ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, V_SIZE_DOUBLE_COMPLEX, false);
    }
#else
    size_t proposed_count;

    // Each rank contribues: TEST_UNIFORM_COUNT elements
    // Largest buffer is   : TEST_UNIFORM_COUNT x world_size
    proposed_count = calc_uniform_count(sizeof(int), TEST_UNIFORM_COUNT,
                                        (size_t)world_size, (size_t)world_size);
    ret += my_c_test_core(MPI_INT, proposed_count * (size_t)world_size, true);

    proposed_count = calc_uniform_count(sizeof(double _Complex), TEST_UNIFORM_COUNT,
                                        (size_t)world_size, (size_t)world_size);
    ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, proposed_count * (size_t)world_size, true);
    if (allow_nonblocked) {
        proposed_count = calc_uniform_count(sizeof(int), TEST_UNIFORM_COUNT,
                                            (size_t)world_size, (size_t)world_size);
        ret += my_c_test_core(MPI_INT, proposed_count * (size_t)world_size, false);
        proposed_count = calc_uniform_count(sizeof(double _Complex), TEST_UNIFORM_COUNT,
                                            (size_t)world_size, (size_t)world_size);
        ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, proposed_count * (size_t)world_size, false);
    }
#endif

    /*
     * All done
     */
    MPI_Finalize();
    return ret;
}

int my_c_test_core(MPI_Datatype dtype, size_t total_num_elements, bool blocking)
{
    int ret = 0;
    size_t i;
    size_t j;

    // Actual payload size as divisible by the sizeof(dt)
    size_t payload_size_actual;
    size_t excess_size_actual;

    int *my_int_recv_vector = NULL;
    int *my_int_send_vector = NULL;
    double _Complex *my_dc_recv_vector = NULL;
    double _Complex *my_dc_send_vector = NULL;
    MPI_Request request;
    int exp;
    size_t num_wrong;
    int excess_count;
    size_t current_base;
    int receive_counts[world_size];
    int receive_offsets[world_size];
    int send_counts[world_size];
    int send_offsets[world_size];
    char *mpi_function = blocking ? "MPI_Alltoallv" : "MPI_Ialltoallv";

    assert(MPI_INT == dtype || MPI_C_DOUBLE_COMPLEX == dtype);
    if (total_num_elements > INT_MAX) {
        total_num_elements = INT_MAX;
    }

    // Force unequal distribution of data across ranks
    if ((total_num_elements % world_size) == 0) {
        total_num_elements = total_num_elements - 1;
    }
    excess_count = total_num_elements % world_size;

    // The value of total_num_elements passed to this function should not exceed
    // INT_MAX. By adding an extra element to force unequal distribution,
    // total_num_elements may exceed INT_MAX so the value must be adjusted
    // downward.
    if ((total_num_elements + excess_count) > INT_MAX) {
        total_num_elements = total_num_elements - world_size;
    }

    // Data sent by all ranks to all ranks other than highest rank is
    // (total_num_elements / world_size) elements. All ranks send that
    // data plus the excess (total_num_elements % world_size) to the
    // highest rank. All ranks must receive exactly the number of elements
    // they were sent.
    current_base = 0;
    for (i = 0; i < world_size; i++) {
        send_counts[i] = total_num_elements / world_size;
        receive_counts[i] = total_num_elements / world_size;
        send_offsets[i] = current_base;
        receive_offsets[i] = current_base;
        current_base = current_base + send_counts[i];
    }
    send_counts[world_size - 1] += excess_count;

    // Since the highest rank receives excess elements due to unequal distribution,
    // the receive counts and receive offsets need to be adjusted by that count.
    if (world_rank == (world_size - 1)) {
        current_base = 0;
        for (i = 0; i < world_size; i++) {
            receive_offsets[i] = current_base;
            receive_counts[i] = (total_num_elements / world_size) + excess_count;
            current_base = current_base + receive_counts[i];
        }
    }

    // Allocate send and receive buffers. The send buffer for each rank is
    // allocated to hold the total_num_elements sent to all ranks. Since
    // total_num_elements is forced to a value not evenly divisible by the
    // world_size, and the excess elements are sent by each rank to the last
    // rank, the receive buffer for the last rank must be larger than the
    // send buffer by excess_count * world_size. For the other ranks, allocating
    // send and receive buffers identically is sufficient.
    if( MPI_INT == dtype ) {
        payload_size_actual = total_num_elements * sizeof(int);
        if (world_rank == (world_size - 1)) {
            excess_size_actual = world_size * excess_count * sizeof(int);
            my_int_recv_vector = (int*)safe_malloc(payload_size_actual +
                                                   excess_size_actual);
        }
        else {
            my_int_recv_vector = (int*)safe_malloc(payload_size_actual);
        }
        my_int_send_vector = (int*)safe_malloc(payload_size_actual);
    } else {
        payload_size_actual = total_num_elements * sizeof(double _Complex);
        if (world_rank == (world_size - 1)) {
            excess_size_actual = world_size * excess_count * sizeof(double _Complex);
            my_dc_recv_vector = (double _Complex*)safe_malloc(payload_size_actual +
                                                  excess_size_actual);
        }
        else {
            my_dc_recv_vector = (double _Complex*)safe_malloc(payload_size_actual);
        }
        my_dc_send_vector = (double _Complex*)safe_malloc(payload_size_actual);
    }

    // Initialize blocks of data to be sent to each rank to a unique range of values
    // using array index modulo prime and offset by prime * rank
    if (MPI_INT == dtype) {
        for (i = 0; i < world_size; ++i) {
            for (j = 0; j < send_counts[i]; j++) {
                exp = (j % PRIME_MODULUS) + (PRIME_MODULUS * world_rank);
                my_int_send_vector[j + send_offsets[i]] = exp;
            }
        }
    }
    else {
        for (i = 0; i < world_size; ++i) {
            for (j = 0; j < send_counts[i]; j++) {
                exp = (j % PRIME_MODULUS) + (PRIME_MODULUS * world_rank);
                my_dc_send_vector[j + send_offsets[i]] = (1.0 * exp - 1.0 * exp * I);
            }
        }
    }

    if (world_rank == 0) {
        printf("---------------------\nResults from %s(%s x %zu = %zu or %s): MPI_IN_PLACE\n",
               mpi_function, (MPI_INT == dtype ? "int" : "double _Complex"),
               total_num_elements, payload_size_actual, human_bytes(payload_size_actual));
    }

    // Perform the MPI_Alltoallv operation
    if (blocking) {
        if( MPI_INT == dtype ) {
            MPI_Alltoallv(my_int_send_vector, send_counts, 
                          send_offsets,       dtype,
                          my_int_recv_vector, receive_counts,
                          receive_offsets,    dtype,
                          MPI_COMM_WORLD);
        } else {
            MPI_Alltoallv(my_dc_send_vector,  send_counts,
                          send_offsets,       dtype,
                          my_dc_recv_vector,  receive_counts,
                          receive_offsets,    dtype,
                          MPI_COMM_WORLD);
        }
    }
    else {
        if( MPI_INT == dtype ) {
            MPI_Ialltoallv(my_int_send_vector, send_counts,
                           send_offsets,       dtype,
                           my_int_recv_vector, receive_counts,
                           receive_offsets,    dtype,
                           MPI_COMM_WORLD,     &request);
        } else {
            MPI_Ialltoallv(my_dc_send_vector,  send_counts,
                           send_offsets,       dtype,
                           my_dc_recv_vector,  receive_counts,
                           receive_offsets,    dtype,
                           MPI_COMM_WORLD,     &request);
        }
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    }

    // Check results. Each receive buffer segment must match the 
    // values in the send buffer segment it was sent.
    num_wrong = 0;
    current_base = 0;
    if (MPI_INT == dtype) {
        for (i = 0; i < world_size; i++) {
            for (j = 0; j < receive_counts[i]; j++) {
                exp = (j % PRIME_MODULUS) + (PRIME_MODULUS * i);
                if (my_int_recv_vector[current_base + j] != exp) {
                    num_wrong = num_wrong + 1;
                }
            }
            current_base = current_base + receive_counts[i];
        }
    }
    else {
        for (i = 0; i < world_size; i++) {
            for (j = 0; j < receive_counts[i]; j++) {
                exp = (j % PRIME_MODULUS) + (PRIME_MODULUS * i);
                if (my_dc_recv_vector[current_base + j] != (1.0 * exp - 1.0 * exp * I)) {
                    num_wrong = num_wrong + 1;
                }
            }
            current_base = current_base + receive_counts[i];
        }
    }

    if (0 == num_wrong) {
        printf("Rank %2d: PASSED\n", world_rank);
    } else {
        printf("Rank %2d: ERROR: DI in %14zu of %14zu slots (%6.1f %% wrong)\n", world_rank,
               num_wrong, total_num_elements,
               ((num_wrong * 1.0) / total_num_elements * 100.0));
        ret = 1;
    }

    if (NULL != my_int_send_vector) {
        free(my_int_send_vector);
    }
    if (NULL != my_int_recv_vector){
        free(my_int_recv_vector);
    }
    if (NULL != my_dc_send_vector) {
        free(my_dc_send_vector);
    }
    if (NULL != my_dc_recv_vector){
        free(my_dc_recv_vector);
    }

    fflush(NULL);
    MPI_Barrier(MPI_COMM_WORLD);

    return ret;
}
