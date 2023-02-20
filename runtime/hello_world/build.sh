#!/bin/bash -e

# Wrapper compiler
_MPICC=mpicc
echo "=========================="
echo "Wrapper compiler: $_MPICC"
echo "=========================="
${_MPICC} --showme

echo "=========================="
echo "Building MPI Hello World"
echo "=========================="
cp ${CI_OMPI_SRC}/examples/hello_c.c .
${_MPICC} hello_c.c -o hello

echo "=========================="
echo "Success"
echo "=========================="
exit 0
