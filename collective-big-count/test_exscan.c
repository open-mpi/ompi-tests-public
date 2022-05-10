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
    ret += my_c_test_core(MPI_INT, V_SIZE_INT, true);
    ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, V_SIZE_DOUBLE_COMPLEX, true);
    if (allow_nonblocked) {
        ret += my_c_test_core(MPI_INT, V_SIZE_INT, false);
        ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, V_SIZE_DOUBLE_COMPLEX, false);
    }
#else
    size_t proposed_count;

    // Each rank contribues: TEST_UNIFORM_COUNT elements
    // Largest buffer is   : TEST_UNIFORM_COUNT elements
    proposed_count = calc_uniform_count(sizeof(int), TEST_UNIFORM_COUNT,
                                        2, 2); // 1 send, 1 recv buffer each
    ret += my_c_test_core(MPI_INT, proposed_count, true);

    proposed_count = calc_uniform_count(sizeof(double _Complex), TEST_UNIFORM_COUNT,
                                        2, 2); // 1 send, 1 recv buffer each
    ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, proposed_count, true);
    if (allow_nonblocked) {
        proposed_count = calc_uniform_count(sizeof(int), TEST_UNIFORM_COUNT,
                                            2, 2); // 1 send, 1 recv buffer each
        ret += my_c_test_core(MPI_INT, proposed_count, false);
        proposed_count = calc_uniform_count(sizeof(double _Complex), TEST_UNIFORM_COUNT,
                                            2, 2); // 1 send, 1 recv buffer each
        ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, proposed_count, false);
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
    MPI_Request request;
    char *mpi_function = blocking ? "MPI_Exscan" : "MPI_Iexscan";

    // Actual payload size as divisible by the sizeof(dt)
    size_t payload_size_actual;

    /*
     * Initialize vector
     */
    int *my_int_recv_vector = NULL;
    int *my_int_send_vector = NULL;
    double _Complex *my_dc_recv_vector = NULL;
    double _Complex *my_dc_send_vector = NULL;
    double _Complex testValue;
    size_t num_wrong = 0;

    assert(MPI_INT == dtype || MPI_C_DOUBLE_COMPLEX == dtype);
    assert(total_num_elements <= INT_MAX);

    if( MPI_INT == dtype ) {
        payload_size_actual = total_num_elements * sizeof(int);
        my_int_recv_vector = (int*)safe_malloc(payload_size_actual);
        my_int_send_vector = (int*)safe_malloc(payload_size_actual);
    } else {
        payload_size_actual = total_num_elements * sizeof(double _Complex);
        my_dc_recv_vector = (double _Complex*)safe_malloc(payload_size_actual);
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
            my_dc_send_vector[i] = (i % PRIME_MODULUS) - (i % PRIME_MODULUS)*I;
        }
    }

    if (world_rank == 0) {
        printf("---------------------\nResults from %s(%s x %zu = %zu or %s):\n",
               mpi_function, (MPI_INT == dtype ? "int" : "double _Complex"),
               total_num_elements, payload_size_actual, human_bytes(payload_size_actual));
    }
    if (blocking) {
        if( MPI_INT == dtype ) {
            MPI_Exscan(my_int_send_vector, my_int_recv_vector,
                       (int)total_num_elements, dtype,
                       MPI_SUM, MPI_COMM_WORLD);
        } else {
            MPI_Exscan(my_dc_send_vector, my_dc_recv_vector,
                       (int)total_num_elements, dtype,
                       MPI_SUM, MPI_COMM_WORLD);
        }
    }
    else {
        if( MPI_INT == dtype ) {
            MPI_Iexscan(my_int_send_vector, my_int_recv_vector,
                        (int)total_num_elements, dtype,
                        MPI_SUM, MPI_COMM_WORLD, &request);
        } else {
            MPI_Iexscan(my_dc_send_vector, my_dc_recv_vector,
                        (int)total_num_elements, dtype,
                        MPI_SUM, MPI_COMM_WORLD, &request);
        }
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    }

    /*
     * Check results.
     * Each output array element must have the value such that
     * out[i] == (in[i] % PRIME_MODULO) * my_rank since out[i] is the sum of in[i]
     * for all * ranks less than our rank and in[i] for all ranks is set to
     * i % PRIME_MODULO
     * Validation is similar to MPI_Scan except
     * 1) Task 0 receive buffer values are indeterminate, so task 0 is not checked.
     * 2) All tasks up to, but not including this task's rank participate in 
     *    setting the values in the receive buffer for this task.
     */
    if (0 != world_rank) {
        for (i = 0; i < total_num_elements; ++i) {
            if( MPI_INT == dtype ) {
                if(my_int_recv_vector[i] != my_int_send_vector[i] * world_rank) {
                    ++num_wrong;
                }
            } else {
                testValue = (i % PRIME_MODULUS) * world_rank -
                            (i % PRIME_MODULUS) * world_rank * I;
                if (my_dc_recv_vector[i] != testValue) {
                    ++num_wrong;
                }
            }
        }
    }

    if( 0 == num_wrong) {
        printf("Rank %2d: PASSED\n", world_rank);
    } else {
        printf("Rank %2d: ERROR: DI in %14zu of %14zu slots (%6.1f %% wrong)\n", world_rank,
               num_wrong, total_num_elements, ((num_wrong * 1.0)/total_num_elements)*100.0);
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
