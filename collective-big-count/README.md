# Big Count Collectives Tests

This test suite is for testing with large **count** payload operations. Large payload is defined as:

 > total payload size (count x sizeof(datatype)) is greater than UINT_MAX (4294967295 =~ 4 GB)

| Test Suite | Count     | Datatype |
| ---------- | -----     | -------- |
| N/A        | small     | small    |
| BigCount   | **LARGE** | small    |
| [BigMPI](https://github.com/jeffhammond/BigMPI) | small | **LARGE**    |

 * Assumes:
   - Roughly the same amount of memory per node
   - Same number of processes per node

## Building

```
make clean
make all
```

## Running

For each unit test two different binaries are generated:
 * `test_FOO` : Run with a total payload size as close to `INT_MAX` as possible relative to the target datatype.
 * `test_FOO_uniform_count` : Run with a uniform count regardless of the datatype. Default `count = 2147483647 (INT_MAX)`

Currently, the unit tests use the `int` and `double _Complex` datatypes in the MPI collectives.

```
mpirun --np 8 --map-by ppr:2:node --host host01:2,host02:2,host03:2,host04:2 \
  -mca coll basic,inter,libnbc,self ./test_allreduce

mpirun --np 8 --map-by ppr:2:node --host host01:2,host02:2,host03:2,host04:2 \
  -x BIGCOUNT_MEMORY_PERCENT=15 -x BIGCOUNT_MEMORY_DIFF=10 \
  --mca coll basic,inter,libnbc,self ./test_allreduce

mpirun --np 8 --map-by ppr:2:node --host host01:2,host02:2,host03:2,host04:2 \
  -x BIGCOUNT_MEMORY_PERCENT=15 -x BIGCOUNT_MEMORY_DIFF=10 \
  --mca coll basic,inter,libnbc,self ./test_allreduce_uniform_count 
```

Expected output will look something like the following. Notice that depending on the `BIGCOUNT_MEMORY_PERCENT` environment variable you might see the collective `Adjust count to fit in memory` message as the test harness is trying to honor that parameter.
```
shell$ mpirun --np 4 --map-by ppr:1:node --host host01,host02,host03,host04 \
  -x BIGCOUNT_MEMORY_PERCENT=6 -x BIGCOUNT_MEMORY_DIFF=10 \
  --mca coll basic,inter,libnbc,self ./test_allreduce_uniform_count
----------------------:-----------------------------------------
Total Memory Avail.   :  567 GB
Percent memory to use :    6 %
Tolerate diff.        :   10 GB
Max memory to use     :   34 GB
----------------------:-----------------------------------------
INT_MAX               :           2147483647
UINT_MAX              :           4294967295
SIZE_MAX              : 18446744073709551615
----------------------:-----------------------------------------
                      : Count x Datatype size      = Total Bytes
TEST_UNIFORM_COUNT    :           2147483647
V_SIZE_DOUBLE_COMPLEX :           2147483647 x  16 =    32.0 GB
V_SIZE_DOUBLE         :           2147483647 x   8 =    16.0 GB
V_SIZE_FLOAT_COMPLEX  :           2147483647 x   8 =    16.0 GB
V_SIZE_FLOAT          :           2147483647 x   4 =     8.0 GB
V_SIZE_INT            :           2147483647 x   4 =     8.0 GB
----------------------:-----------------------------------------
---------------------
Results from MPI_Allreduce(int x 2147483647 = 8589934588 or 8.0 GB):
Rank  3: PASSED
Rank  2: PASSED
Rank  1: PASSED
Rank  0: PASSED
--------------------- Adjust count to fit in memory: 2147483647 x  50.0% = 1073741823
Root  : payload    34359738336  32.0 GB =  16 dt x 1073741823 count x   2 peers x   1.0 inflation
Peer  : payload    34359738336  32.0 GB =  16 dt x 1073741823 count x   2 peers x   1.0 inflation
Total : payload    34359738336  32.0 GB =  32.0 GB root +  32.0 GB x   0 local peers
---------------------
Results from MPI_Allreduce(double _Complex x 1073741823 = 17179869168 or 16.0 GB):
Rank  3: PASSED
Rank  2: PASSED
Rank  0: PASSED
Rank  1: PASSED
---------------------
Results from MPI_Iallreduce(int x 2147483647 = 8589934588 or 8.0 GB):
Rank  2: PASSED
Rank  0: PASSED
Rank  3: PASSED
Rank  1: PASSED
--------------------- Adjust count to fit in memory: 2147483647 x  50.0% = 1073741823
Root  : payload    34359738336  32.0 GB =  16 dt x 1073741823 count x   2 peers x   1.0 inflation
Peer  : payload    34359738336  32.0 GB =  16 dt x 1073741823 count x   2 peers x   1.0 inflation
Total : payload    34359738336  32.0 GB =  32.0 GB root +  32.0 GB x   0 local peers
---------------------
Results from MPI_Iallreduce(double _Complex x 1073741823 = 17179869168 or 16.0 GB):
Rank  2: PASSED
Rank  0: PASSED
Rank  3: PASSED
Rank  1: PASSED
```

## Environment variables

 * `BIGCOUNT_MEMORY_DIFF` (Default: `0`): Maximum difference (as integer in GB) in total available memory between processes.
 * `BIGCOUNT_MEMORY_PERCENT` (Default: `80`): Maximum percent (as integer) of memory to consume.
 * `BIGCOUNT_ENABLE_NONBLOCKING` (Default: `1`): Enable/Disable the nonblocking collective tests. `y`/`Y`/`1` means Enable, otherwise disable.
 * `BIGCOUNT_ALG_INFLATION` (Default: `1.0`): Memory overhead multiplier for a given algorithm. Some algorithms use internal buffers relative to the size of the payload and/or communicator size. This envar allow you to account for that to help avoid Out-Of-Memory (OOM) scenarios.

## Missing Collectives (to do list)

Collectives missing from this test suite:
  * Barrier (N/A)
  * Alltoallv
  * Reduce_scatter
  * Scan / Exscan
