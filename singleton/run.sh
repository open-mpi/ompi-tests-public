#!/bin/bash -e

echo "====================="
echo "Testing: Hello with mpirun"
echo "====================="
mpirun --np 1 ./hello_c

echo "====================="
echo "Testing: Hello as a singleton"
echo "====================="
./hello_c


echo "====================="
echo "Testing: MPI_Comm_spawn with mpirun"
echo "====================="
mpirun --np 1 ./simple_spawn ./simple_spawn

echo "====================="
echo "Testing: MPI_Comm_spawn as a singleton"
echo "====================="
./simple_spawn ./simple_spawn


echo "====================="
echo "Testing: MPI_Comm_spawn_multiple with mpirun"
echo "====================="
mpirun --np 1 ./simple_spawn_multiple ./simple_spawn_multiple

echo "====================="
echo "Testing: MPI_Comm_spawn_multiple as a singleton"
echo "====================="
./simple_spawn_multiple ./simple_spawn_multiple


echo "====================="
echo "Success"
echo "====================="
