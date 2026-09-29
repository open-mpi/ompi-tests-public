#!/bin/bash -e

echo "============================="
echo "Testing: MPI_Win_shared_query"
echo "============================="
mpirun --np 2 ./test_mpi_win_shared_query

