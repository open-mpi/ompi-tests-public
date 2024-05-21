// Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
// (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
// Government retains certain rights in this software.

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h> 
#include "mpi.h" 

#define CHECK_RETVAL(x) do { \
    int retval = (x); \
    if (retval != MPI_SUCCESS) { \
        fprintf(stderr, "Error: %s returned %d (expected %d)\n", #x, retval, MPI_SUCCESS); \
        MPI_Abort(MPI_COMM_WORLD, 999); \
        } \
    } while (0)

#define TEST_RAN_TO_COMPLETION() do { \
    fprintf(stderr, "END\n"); \
    } while (0)

#endif
