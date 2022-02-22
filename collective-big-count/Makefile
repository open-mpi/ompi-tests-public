#
#  Copyright (c) 2021-2022 IBM Corporation.  All rights reserved.
#
# $COPYRIGHT$
#

######################################################################
# Utilities
######################################################################
.PHONY: default help

CC	= mpicc
F77	= mpif77
F90	= mpif90
MPIRUN = mpirun
RM	= /bin/rm -f

# GCC
CC_FLAGS = -g -O0 -Wall -Werror
# Clang
#CC_FLAGS = -g -O0 -Wall -Wshorten-64-to-32 -Werror
F90_FLAGS =
F77_FLAGS = $(F90_FLAGS)

######################################################################
# TEST_UNIFORM_COUNT: (Default defined here)
#  The 'count' size to be used regardless of the datatype
#  This should never exceed that of INT_MAX (2147483647) which
#  is the maximum count allowed by the MPI Interface in MPI 3
######################################################################
# Test at the limit of INT_MAX : 2147483647
TEST_UNIFORM_COUNT=2147483647

######################################################################
# TEST_PAYLOAD_SIZE: (Default in common.h)
#  This value is the total payload size the collective should perform.
#  The 'count' is calculated as relative to the datatype size so
#  as to target this payload size as closely as possible:
#  count = TEST_PAYLOAD_SIZE / sizeof(datatype)
######################################################################
# INT_MAX    : == 2 GB so guard will not trip (INT_MAX == 2GB -1byte)
TEST_PAYLOAD_SIZE=2147483647

######################################################################
# Binaries
######################################################################
BINCC	= \
	test_alltoall \
	test_allgather test_allgatherv \
	test_allreduce \
	test_bcast \
	test_gather test_gatherv \
	test_reduce \
	test_scatter test_scatterv \
	diagnostic

BIN	= $(BINCC)

######################################################################
# Targets
######################################################################
all: $(BIN)

clean:
	$(RM) $(BIN) *.o *_uniform_count *_uniform_payload

diagnostic: common.h diagnostic.c
	$(CC) $(CC_FLAGS) -DTEST_PAYLOAD_SIZE=$(TEST_PAYLOAD_SIZE) -o $@ -I. diagnostic.c
	$(CC) $(CC_FLAGS) -DTEST_UNIFORM_COUNT=$(TEST_UNIFORM_COUNT) -o $@_uniform_count -I. diagnostic.c

test_allgather: common.h test_allgather.c
	$(CC) $(CC_FLAGS) -DTEST_PAYLOAD_SIZE=$(TEST_PAYLOAD_SIZE) -o $@ -I. test_allgather.c
	$(CC) $(CC_FLAGS) -DTEST_UNIFORM_COUNT=$(TEST_UNIFORM_COUNT) -o $@_uniform_count -I. test_allgather.c

test_allgatherv: common.h test_allgatherv.c
	$(CC) $(CC_FLAGS) -DTEST_PAYLOAD_SIZE=$(TEST_PAYLOAD_SIZE) -o $@ -I. test_allgatherv.c
	$(CC) $(CC_FLAGS) -DTEST_UNIFORM_COUNT=$(TEST_UNIFORM_COUNT) -o $@_uniform_count -I. test_allgatherv.c

test_allreduce: common.h test_allreduce.c
	$(CC) $(CC_FLAGS) -DTEST_PAYLOAD_SIZE=$(TEST_PAYLOAD_SIZE) -o $@ -I. test_allreduce.c
	$(CC) $(CC_FLAGS) -DTEST_UNIFORM_COUNT=$(TEST_UNIFORM_COUNT) -o $@_uniform_count -I. test_allreduce.c

test_alltoall: common.h test_alltoall.c
	$(CC) $(CC_FLAGS) -DTEST_PAYLOAD_SIZE=$(TEST_PAYLOAD_SIZE) -o $@ -I. test_alltoall.c
	$(CC) $(CC_FLAGS) -DTEST_UNIFORM_COUNT=$(TEST_UNIFORM_COUNT) -o $@_uniform_count -I. test_alltoall.c

test_bcast: common.h test_bcast.c
	$(CC) $(CC_FLAGS) -DTEST_PAYLOAD_SIZE=$(TEST_PAYLOAD_SIZE) -o $@ -I. test_bcast.c
	$(CC) $(CC_FLAGS) -DTEST_UNIFORM_COUNT=$(TEST_UNIFORM_COUNT) -o $@_uniform_count -I. test_bcast.c

test_gather: common.h test_gather.c
	$(CC) $(CC_FLAGS) -DTEST_PAYLOAD_SIZE=$(TEST_PAYLOAD_SIZE) -o $@ -I. test_gather.c
	$(CC) $(CC_FLAGS) -DTEST_UNIFORM_COUNT=$(TEST_UNIFORM_COUNT) -o $@_uniform_count -I. test_gather.c

test_gatherv: common.h test_gatherv.c
	$(CC) $(CC_FLAGS) -DTEST_PAYLOAD_SIZE=$(TEST_PAYLOAD_SIZE) -o $@ -I. test_gatherv.c
	$(CC) $(CC_FLAGS) -DTEST_UNIFORM_COUNT=$(TEST_UNIFORM_COUNT) -o $@_uniform_count -I. test_gatherv.c

test_reduce: common.h test_reduce.c
	$(CC) $(CC_FLAGS) -DTEST_PAYLOAD_SIZE=$(TEST_PAYLOAD_SIZE) -o $@ -I. test_reduce.c
	$(CC) $(CC_FLAGS) -DTEST_UNIFORM_COUNT=$(TEST_UNIFORM_COUNT) -o $@_uniform_count -I. test_reduce.c

test_scatter: common.h test_scatter.c
	$(CC) $(CC_FLAGS) -DTEST_PAYLOAD_SIZE=$(TEST_PAYLOAD_SIZE) -o $@ -I. test_scatter.c
	$(CC) $(CC_FLAGS) -DTEST_UNIFORM_COUNT=$(TEST_UNIFORM_COUNT) -o $@_uniform_count -I. test_scatter.c

test_scatterv: common.h test_scatterv.c
	$(CC) $(CC_FLAGS) -DTEST_PAYLOAD_SIZE=$(TEST_PAYLOAD_SIZE) -o $@ -I. test_scatterv.c
	$(CC) $(CC_FLAGS) -DTEST_UNIFORM_COUNT=$(TEST_UNIFORM_COUNT) -o $@_uniform_count -I. test_scatterv.c
