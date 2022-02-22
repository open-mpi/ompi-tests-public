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

int my_c_test_core(MPI_Datatype dtype, size_t total_num_elements, bool in_place,
                   bool blocking);

int main(int argc, char** argv) {
    /*
     * Initialize the MPI environment
     */
    int ret = 0;

    MPI_Init(NULL, NULL);
    init_environment(argc, argv);

    // Run the tests
#ifndef TEST_UNIFORM_COUNT
    // Each rank contribues: V_SIZE_INT / world_size elements
    // Largest buffer is   : V_SIZE_INT elements
    ret += my_c_test_core(MPI_INT, V_SIZE_INT, false, true);
    ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, V_SIZE_DOUBLE_COMPLEX, false, true);
    if (allow_nonblocked) {
        ret += my_c_test_core(MPI_INT, V_SIZE_INT, false, false);
        ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, V_SIZE_DOUBLE_COMPLEX, false, false);
    }
#else
    size_t proposed_count;

    // Each rank contribues: TEST_UNIFORM_COUNT elements
    // Largest buffer is   : TEST_UNIFORM_COUNT x world_size
    proposed_count = calc_uniform_count(sizeof(int), TEST_UNIFORM_COUNT,
                                        (size_t)world_size, (size_t)world_size, alg_inflation);
    ret += my_c_test_core(MPI_INT, proposed_count * (size_t)world_size, true, true);
    proposed_count = calc_uniform_count(sizeof(double _Complex), TEST_UNIFORM_COUNT,
                                        (size_t)world_size, (size_t)world_size, alg_inflation);
    ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, proposed_count * (size_t)world_size, true, true);
    if (allow_nonblocked) {
        proposed_count = calc_uniform_count(sizeof(int), TEST_UNIFORM_COUNT,
                                            (size_t)world_size, (size_t)world_size, alg_inflation);
        ret += my_c_test_core(MPI_INT, proposed_count * (size_t)world_size, true, false);
        proposed_count = calc_uniform_count(sizeof(double _Complex), TEST_UNIFORM_COUNT,
                                            (size_t)world_size, (size_t)world_size, alg_inflation);
        ret += my_c_test_core(MPI_C_DOUBLE_COMPLEX, proposed_count * (size_t)world_size, true,
                              false);
    }
#endif

    /*
     * All done
     */
    MPI_Finalize();
    return ret;
}

int my_c_test_core(MPI_Datatype dtype, size_t total_num_elements, bool in_place, bool blocking)
{
    int ret = 0;
    size_t i;

    // Actual payload size as divisible by the sizeof(dt)
    size_t payload_size_actual;

    /*
     * Initialize vector
     */
    int *my_int_send_vector = NULL;
    int *my_int_recv_vector = NULL;
    double _Complex *my_dc_send_vector = NULL;
    double _Complex *my_dc_recv_vector = NULL;
    size_t recv_count = 0;
    size_t send_count = 0;
    int exp;
    size_t num_wrong = 0;
    MPI_Request request;
    char *mpi_function = blocking ? "MPI_Allgather" : "MPI_Iallgather";

    assert(MPI_INT == dtype || MPI_C_DOUBLE_COMPLEX == dtype);

    send_count = recv_count = total_num_elements / (size_t)world_size;
    // total_num_elements must be a multiple of world_size. Drop any remainder
    total_num_elements = send_count * (size_t)world_size;

    if( MPI_INT == dtype ) {
        payload_size_actual = total_num_elements * sizeof(int);
        my_int_recv_vector = (int*)safe_malloc(payload_size_actual);
        if( !in_place ) {
            my_int_send_vector = (int*)safe_malloc(send_count * sizeof(int));
        }
    } else {
        payload_size_actual = total_num_elements * sizeof(double _Complex);
        my_dc_recv_vector = (double _Complex*)safe_malloc(payload_size_actual);
        if( !in_place ) {
            my_dc_send_vector = (double _Complex*)safe_malloc(send_count * sizeof(double _Complex));
        }
    }

    if( in_place ) {
        for(i = 0; i < total_num_elements; ++i) {
            exp = 1 + world_rank;
            if( MPI_INT == dtype ) {
                if( (i / (size_t)send_count) == world_rank ) {
                    my_int_recv_vector[i] = exp;
                } else {
                    my_int_recv_vector[i] = -1;
                }
            } else {
                if( (i / (size_t)send_count) == world_rank ) {
                    my_dc_recv_vector[i] = 1.0*exp - 1.0*exp*I;
                } else {
                    my_dc_recv_vector[i] = 1.0 + 1.0*I;
                }
            }
        }
    } else {
        for(i = 0; i < send_count; ++i) {
            exp = 1 + world_rank;
            if( MPI_INT == dtype ) {
                my_int_send_vector[i] = exp;
            } else {
                my_dc_send_vector[i] = 1.0*exp - 1.0*exp*I;
            }
        }
        for(i = 0; i < total_num_elements; ++i) {
            if( MPI_INT == dtype ) {
                my_int_recv_vector[i] = -1;
            } else {
                my_dc_recv_vector[i] = 1.0 + 1.0*I;
            }
        }
    }

    if (world_rank == 0) {
        printf("---------------------\nResults from %s(%s x %zu = %zu or %s): %s\n",
               mpi_function, (MPI_INT == dtype ? "int" : "double _Complex"),
               total_num_elements, payload_size_actual, human_bytes(payload_size_actual),
               ((in_place) ? " MPI_IN_PLACE" : ""));
    }
    if(!in_place) {
        assert(send_count <= INT_MAX);
    }
    assert(recv_count <= INT_MAX);
    if (blocking) {
        if( MPI_INT == dtype ) {
            MPI_Allgather(in_place ? MPI_IN_PLACE : my_int_send_vector, (int)send_count, dtype,
                          my_int_recv_vector, (int)recv_count, dtype,
                          MPI_COMM_WORLD);
        } else {
            MPI_Allgather(in_place ? MPI_IN_PLACE : my_dc_send_vector, (int)send_count, dtype,
                          my_dc_recv_vector, (int)recv_count, dtype,
                          MPI_COMM_WORLD);
        }
    }
    else {
        if( MPI_INT == dtype ) {
            MPI_Iallgather(in_place ? MPI_IN_PLACE : my_int_send_vector, (int)send_count, dtype,
                          my_int_recv_vector, (int)recv_count, dtype,
                          MPI_COMM_WORLD, &request);
        } else {
            MPI_Iallgather(in_place ? MPI_IN_PLACE : my_dc_send_vector, (int)send_count, dtype,
                          my_dc_recv_vector, (int)recv_count, dtype,
                          MPI_COMM_WORLD, &request);
        }
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    }

    /*
     * Check results.
     */
    exp = 0;
    for(i = 0; i < total_num_elements; ++i) {
        exp = (int)(1 + (i / (size_t)recv_count));
        if( MPI_INT == dtype ) {
            if(my_int_recv_vector[i] != exp) {
                ++num_wrong;
            }
        } else {
            if(my_dc_recv_vector[i] != 1.0*exp - 1.0*exp*I) {
                ++num_wrong;
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
