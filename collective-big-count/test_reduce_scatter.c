/*
 * Copyright (c) 2021 IBM Corporation.  All rights reserved.
 *
 * $COPYRIGHT$
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mpi.h>
#include "common.h"

int my_c_test_core(MPI_Datatype dtype, size_t total_num_elements,
                   int world_size, bool blocking);

int main(int argc, char** argv) {
    /*
     * Initialize the MPI environment
     */
    int ret = 0;

    MPI_Init(NULL, NULL);
    init_environment(argc, argv);

    // Run the tests
#ifndef TEST_UNIFORM_COUNT
    // Each rank contribues: V_SIZE_INT elements
    // Largest buffer is   : V_SIZE_INT elements
    ret += my_c_test_core(MPI_INT, V_SIZE_INT, world_size, true);
    ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, V_SIZE_DOUBLE_COMPLEX,
                          world_size, true);
    if (allow_nonblocked) {
        ret += my_c_test_core(MPI_INT, V_SIZE_INT, world_size, false);
        ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, V_SIZE_DOUBLE_COMPLEX,
                              world_size, false);
    }
#else
    size_t proposed_count;

    // Each rank contribues: TEST_UNIFORM_COUNT elements
    // Largest buffer is   : TEST_UNIFORM_COUNT elements
    proposed_count = calc_uniform_count(sizeof(int), TEST_UNIFORM_COUNT,
                                        2, 2); // 1 send, 1 recv buffer each
    ret += my_c_test_core(MPI_INT, proposed_count, world_size, true);

    proposed_count = calc_uniform_count(sizeof(double _Complex), TEST_UNIFORM_COUNT,
                                        2, 2); // 1 send, 1 recv buffer each
    ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, proposed_count, world_size,
                          true);
    if (allow_nonblocked) {
        proposed_count = calc_uniform_count(sizeof(int), TEST_UNIFORM_COUNT,
                                            2, 2); // 1 send, 1 recv buffer each
        ret += my_c_test_core(MPI_INT, proposed_count, world_size, false);
        proposed_count = calc_uniform_count(sizeof(double _Complex), TEST_UNIFORM_COUNT,
                                            2, 2); // 1 send, 1 recv buffer each
        ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, proposed_count,
                              world_size, false);
    }
#endif

    /*
     * All done
     */
    MPI_Finalize();
    return ret;
}

int my_c_test_core(MPI_Datatype dtype, size_t total_num_elements,
                   int world_size, bool blocking)
{
    int ret = 0;
    size_t i;
    int count_for_task[world_size];
    size_t in_lbound;
    MPI_Request request;
    char *mpi_function = blocking ? "MPI_Reduce_scatter" : "MPI_Ireduce_scatter";

    // Actual payload size as divisible by the sizeof(dt)
    size_t payload_size_actual;
    size_t recv_size_actual;

    // Assign same number of results to each task, last task gets excess
    for (i = 0; i < world_size; i++) {
        count_for_task[i] = total_num_elements / world_size;
    }
    count_for_task[world_size - 1] += total_num_elements % world_size;

    /*
     * Initialize vector
     */
    int *my_int_recv_vector = NULL;
    int *my_int_send_vector = NULL;
    double _Complex *my_dc_recv_vector = NULL;
    double _Complex *my_dc_send_vector = NULL;
    size_t num_wrong = 0;

    assert(MPI_INT == dtype || MPI_C_DOUBLE_COMPLEX == dtype);
    assert(total_num_elements <= INT_MAX);

    if( MPI_INT == dtype ) {
        payload_size_actual = total_num_elements * sizeof(int);
        recv_size_actual = count_for_task[world_rank] * sizeof(int);
        my_int_recv_vector = (int*)safe_malloc(recv_size_actual);
        my_int_send_vector = (int*)safe_malloc(payload_size_actual);
    } else {
        payload_size_actual = total_num_elements * sizeof(double _Complex);
        recv_size_actual = count_for_task[world_rank] * sizeof(double _Complex);
        my_dc_recv_vector = (double _Complex*)safe_malloc(recv_size_actual);
        my_dc_send_vector = (double _Complex*)safe_malloc(payload_size_actual);
    }

    /*
     * Assign each input array element the value of its array index modulo some
     * prime as an attempt to assign unique values to each array elements and catch
     * errors where array elements get updated with wrong values. Use a prime
     * number in order to avoid problems related to powers of 2.
     */
    for(i = 0; i < total_num_elements; ++i) {
        if( MPI_INT == dtype ) {
            my_int_send_vector[i] = i % PRIME_MODULUS;
        } else {
            my_dc_send_vector[i] = i * PRIME_MODULUS - i * PRIME_MODULUS*I;
        }
    }
    for(i = 0; i < count_for_task[world_rank]; ++i) {
        if( MPI_INT == dtype ) {
            my_int_recv_vector[i] = -1;
        } else {
            my_dc_recv_vector[i] = -1.0 + 1.0*I;
        }
    }

    if (world_rank == 0) {
        printf("---------------------\nResults from %s(%s x %zu = %zu or %s):\n",
               mpi_function, (MPI_INT == dtype ? "int" : "double _Complex"),
               total_num_elements, payload_size_actual, human_bytes(payload_size_actual));
    }
    if (blocking) {
        if( MPI_INT == dtype ) {
            MPI_Reduce_scatter(my_int_send_vector, my_int_recv_vector,
                               count_for_task, dtype,
                               MPI_SUM, MPI_COMM_WORLD);
        } else {
            MPI_Reduce_scatter(my_dc_send_vector, my_dc_recv_vector,
                               count_for_task, dtype,
                               MPI_SUM, MPI_COMM_WORLD);
        }
    }
    else {
        if( MPI_INT == dtype ) {
            MPI_Ireduce_scatter(my_int_send_vector, my_int_recv_vector,
                                count_for_task, dtype,
                                MPI_SUM, MPI_COMM_WORLD, &request);
        } else {
            MPI_Ireduce_scatter(my_dc_send_vector, my_dc_recv_vector,
                                count_for_task, dtype,
                                MPI_SUM, MPI_COMM_WORLD, &request);
        }
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    }

    /*
     * Check results.
     * The reduce-scatter operation performs a reduction (sum) for all elements 
     * of the input array then scatters the reduction result such that each task
     * gets the number of elements specified by count_for_task[world_rank].
     * The input array element for each task is set to the value of its array index
     * modulo a prime number, so the output value for each array element must be 
     * the corresponding input array element value * number of tasks in the application.
     */
    in_lbound = (total_num_elements / world_size) * world_rank;
    for (i = 0; i < count_for_task[world_rank]; ++i) {
        if (MPI_INT == dtype) {
            if (my_int_recv_vector[i] != my_int_send_vector[in_lbound + i] * world_size) {
                ++num_wrong;
            }
        }
        else {
            if (my_dc_recv_vector[i] != my_dc_send_vector[in_lbound + i] * world_size) {
                ++num_wrong;
            }
        }
    }

    if( 0 == num_wrong) {
        printf("Rank %2d: PASSED\n", world_rank);
    } else {
        printf("Rank %2d: ERROR: DI in %14zu of %14u slots (%6.1f %% wrong)\n", world_rank,
               num_wrong, count_for_task[world_rank],
               ((num_wrong * 1.0) / count_for_task[world_rank])*100.0);
        ret = 1;
    }

    if( NULL != my_int_send_vector ) {
        free(my_int_send_vector);
    }
    if( NULL != my_int_recv_vector ){
        free(my_int_recv_vector);
    }
    if( NULL != my_dc_send_vector ) {
        free(my_dc_send_vector);
    }
    if( NULL != my_dc_recv_vector ){
        free(my_dc_recv_vector);
    }

    fflush(NULL);
    MPI_Barrier(MPI_COMM_WORLD);

    return ret;
}
