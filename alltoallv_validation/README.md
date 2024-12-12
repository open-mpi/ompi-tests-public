# Alltoallv Validation of complex datatypes

This test creates a variety of configurations for testing data validation of
the alltoallv collective using non-standard datatypes.

The approach is the following sequence:
 - Create some datatype
 - Determine the packed size, and allocate both packed and unpacked buffers to
   hold the send data.
 - Fill the packed buffer with a test pattern, then sendrecv it to the unpacked
   send buffer by sending from a MPI_BYTES buffer to the test datatype.
 - Perform the alltoallv collective
 - Transfer the received data back into a packed format.
 - Verify the contents of the packed format using knowledge of what data was
   being sent.
 - Verify that no buffer under-runs or over-runs occured in the buffers by
   checking some guard bytes.

Validation is the only purpose of this test.  It should not be used for
performance timing, as many extra memory copies and assignments are performed.
No timing is printed.

The code is written in C++ only to access a predictable random number generator.
All MPI calls are done via C interface.

## Test Overview

Tests are broken into complexity levels.

### Level 1

Level 1 types are composed of basic MPI types like `MPI_CHAR`, `MPI_REAL`,
`MPI_INT64_T` and so forth.  The data types are not exhaustive, only 9 are used.
Executing only the level 1 tests will perform only 9 tests: both sending and
receiving the same datatype.

### Level 2

Level 2 types are collections of Level 1 types.  There are 7 Level 1 types in
various configurations including:

 - increasing the count, using the same type
 - contiguous and non-contiguous vectors
 - contiguous and non-contiguous vectors with negative stride

Level 2 tests all exchange compatible types, therefore all combinations of the
above are used as send and receive types.  With 7 types, Level 2 executes 49
tests.

All level 2 tests are performed with the same basic datatype (MPI_INT).

Note that each "one" of these types is a vector, so setting `--item-count` to 10
really means you are sending 10 vectors each with some number (happens to be 12)
of basic types.

### Level 3

Level 3 tests collections of two different Level 1 types.  We test MPI_INT and
MPI_CHAR together.  These tests create the type using MPI_Type_create_struct in
various orders and configurations including:
 - contiguous and non-contiguous in-order elements
 - contiguous and non-contiguous reverse-order elements
 - Negative lower bounds
 - Padding in extents

There are 6 Level 3 tests, and like Level 2 tests they are all compatible types,
so 36 total tests are executed.

### Level 4

There are two hand-made Level 4 tests.  These are composed of several layers of
level 2 and level 3 types in combination with each other to make collections of
different kinds of types in vectors with various paddings and spacings.  Best to
read the code for these.  They are not cross-compatible, so only 2 tests are
executed.

Again note that these constructed tytes are somewhat large themselves (hundreds
of bytes), so setting a high `--item-count` could result in longer runtimes.

### Total

As of the initial version of this program, there were 96 tests.  The
configuration where all ranks send and receive 1 count for only 1 iteration
results in each rank sending and receiving approximately 2.7KBytes of data per
rank during the full test battery.

However there is not so much data that the execution time is unreasonable.  Test
execution of 32 ranks on a single host using all default options takes less than
5 seconds, and most ranks send about 630 KBytes.

# Compile
```
$ ./autogen.sh && ./configure && make

$ mpirun -n 13 src/alltoallv_ddt
Rank 0 sent 254104 bytes, and received 265152 bytes.
[OK] All tests passsed.  Executed 96 tests with seed 0 with 13 total ranks).

```

# Usage
```
Test alltoallv using various ddt's and validate results.
This test uses pseudo-random sequences from C++'s mt19937 generator.
The test (but not necessarily the implementation) is deterministic
when the options and number of ranks remain the same.
Options:
         [-s|--seed <seed>]           Change the seed to shuffle which datapoints are exchanged
         [-c|--item-count <citems>]   Each rank will create <citems> to consider for exchange (default=10).
         [-i|--prob-item <prob>]      Probability that rank r will send item k to rank q. (0.50)
         [-r|--prob-rank <prob>]      Probability that rank r will send anything to rank q. (0.90)
         [-w|--prob-world <prob>]     Probability that rank r will do anything at all. (0.95)
         [-t|--iters <iters>]         The number of iterations to test each dtype.
         [-o|--only <high,low>]       Only execute a specific test signified by the pair high,low.
         [-v|--verbose=level ]        Set verbosity during execution (0=quiet (default). 1,2,3: loud).
         [-h|--help]                  Print this help and exit.
         [-z|--verbose-rank]          Only the provided rank will print.  Default=0.  ALL = -1.
```

Some recommended test cases:
```
# no ranks exchange any data
alltoallv_ddt -w 0

# same as alltoall: all ranks exchange same amount of data
alltoallv_ddt -w 1 -r 1 -i 1

# perform a different test each time you run, or repeat the same test:
alltoallv_ddt -s $RANDOM
alltoallv_ddt -s 1234
```

Note since alltoall is a hefty collective, and we go to the trouble of
validating every single message, caution should be used when exercising large
numbers of ranks, large numbers of counts, or large numbers of iterations.

# Debugging

In the case of data validation failure: re-run the test harness on only the
failing test (using `--only` and increase the verbosity up to 3.  You may also
need to set the verbosity of a particular rank with `-z`).

For example at verbosity 0, we only know that validation failed on rank 1, but
not which test.

```
mpirun -n 2 src/alltoallv_ddt -z 1 -v 3 -w 1
Rank 1 failed to validate data!
ERROR: Validation failed on rank 1!
```

Setting the rank-specific verbosity to that rank (or to all ranks) and the
verbosity up to 2 reveals some additional details including which test, and what
part of the buffer:

```
$ mpirun -n 2 src/alltoallv_ddt -z 1 -v 3 -w 1
--- Starting test 2,1.  Crossing 0 x 0
Rank 1 failed to validate data!
0010:  42-42   99-43   44-44   45-45   46-46   47-47   48-48   49-49   50-50   51-51  -- CORRUPT
0020:  52-52   53-53   54-54   55-55   56-56   57-57   58-58   59-59   60-60   61-61  -- VALID
ERROR: Validation failed on rank 1!
```

Buffer addresses are provided.  These are base-10 addresses relative to the
packed representation of the datatype.  The first number is what was received,
the second number is what was expected.  To avoid too much print-outs,
subsequent CORRUPT lines are skipped and only the next valid line is printed, so
output will allways appear to alternate between CORRUPT and VALID.
