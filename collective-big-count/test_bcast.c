/*
 * Copyright (c) 2021-2022 IBM Corporation.  All rights reserved.
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
                                        2, 2, 1.0); // 1 send, 1 recv buffer each
    ret += my_c_test_core(MPI_INT, proposed_count, true);
    proposed_count = calc_uniform_count(sizeof(double _Complex), TEST_UNIFORM_COUNT,
                                        2, 2, 1.0); // 1 send, 1 recv buffer each
    ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, proposed_count, true);
    if (allow_nonblocked) {
        proposed_count = calc_uniform_count(sizeof(int), TEST_UNIFORM_COUNT,
                                            2, 2, 1.0); // 1 send, 1 recv buffer each
        ret += my_c_test_core(MPI_INT, proposed_count, false);
        proposed_count = calc_uniform_count(sizeof(double _Complex), TEST_UNIFORM_COUNT,
                                            2, 2, 1.0); // 1 send, 1 recv buffer each
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
    int i;
    MPI_Request request;
    char *mpi_function = blocking ? "MPI_Bcast" : "MPI_Ibcast";

    // Actual payload size as divisible by the sizeof(dt)
    size_t payload_size_actual;

    /*
     * Initialize vector
     */
    int *my_int_vector = NULL;
    double _Complex *my_dc_vector = NULL;
    void *buff_ptr = NULL;
    unsigned int num_wrong = 0;

    assert(MPI_INT == dtype || MPI_C_DOUBLE_COMPLEX == dtype);
    assert(total_num_elements <= INT_MAX);

    if( MPI_INT == dtype ) {
        payload_size_actual = total_num_elements * sizeof(int);
        my_int_vector = (int*)safe_malloc(payload_size_actual);
        buff_ptr = my_int_vector;
    } else {
        payload_size_actual = total_num_elements * sizeof(double _Complex);
        my_dc_vector = (double _Complex*)safe_malloc(payload_size_actual);
        buff_ptr = my_dc_vector;
    }

    for(i = 0; i < total_num_elements; ++i) {
        if( MPI_INT == dtype ) {
            if (world_rank == 0) {
                my_int_vector[i] = i;
            } else {
                my_int_vector[i] = -1;
            }
        } else {
            if (world_rank == 0) {
                my_dc_vector[i] = 1.0*i - 1.0*i*I;
            } else {
                my_dc_vector[i] = -1.0 - 1.0*I;
            }
        }
    }

    if (world_rank == 0) {
        printf("---------------------\nResults from %s(%s x %zu = %zu or %s):\n",
               mpi_function, (MPI_INT == dtype ? "int" : "double _Complex"),
               total_num_elements, payload_size_actual, human_bytes(payload_size_actual));
    }
    if (blocking) {
        MPI_Bcast(buff_ptr, (int)total_num_elements, dtype, 0, MPI_COMM_WORLD);
    }
    else {
        MPI_Ibcast(buff_ptr, (int)total_num_elements, dtype, 0, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    }

    /*
     * Check results.
     */
    for(i = 0; i < total_num_elements; ++i) {
        if( MPI_INT == dtype ) {
            if(my_int_vector[i] != i) {
                ++num_wrong;
            }
        } else {
            if(my_dc_vector[i] != 1.0*i - 1.0*i*I) {
                ++num_wrong;
            }
        }
    }

    if( 0 == num_wrong) {
        printf("Rank %2d: Passed\n", world_rank);
    } else {
        printf("Rank %2d:  ERROR: DI in %14u of %14zu slots (%6.1f %% wrong)\n", world_rank,
               num_wrong, total_num_elements, ((num_wrong * 1.0)/total_num_elements)*100.0);
        ret = 1;
    }

    if(NULL != my_int_vector) {
        free(my_int_vector);
    }
    if(NULL != my_dc_vector) {
        free(my_dc_vector);
    }
    fflush(NULL);
    MPI_Barrier(MPI_COMM_WORLD);

    return ret;
}
