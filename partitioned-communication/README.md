## A collection of tests for the MPI 4.1 partitioned communication API

These tests target partitioned communication as it appears in Chapter 4 of 
the [MPI Specification v. 4.1](https://www.mpi-forum.org/docs/).

The repository contains the following:
- `test_*.c`    : the tests (see below)
- `runtests.py` : a simple python 3 script for executing tests and generating a report
- other misc files : skeleton makefile, license, etc.

### runtests.py script

The script executes each test and looks for an expected output string in its 
`stderr` output. Tests pass if the expected string is found somewhere in the 
output, or (optionally) on a specific line of the output. Alternatively a test 
may be designed to elicit a timeout, in which case it passes if a timeout occurs.

A report is generated in a subdirectory along with the stderr output of each test.

Run with `-v` flag to generate progress information on `stdout`; otherwise the tests 
run silently.

See comments in script for more information.

### Tests

A summary of tests is given below. For each, see the comments in the source for 
motivation for the test, including (when available) references to specific pages and lines in the 
v4.1 MPI specification.

Some tests are designed to elicit errors from the MPI implementation, and if no 
error is produced, the test fails. In some cases the assumption that the implementation 
will produce an error message may be unfounded, e.g., because performing such a check is 
difficult, impacts performance, or it is really up to the user to avoid the situation. 
These tests may warrant additional discussion or exclusion. 

| Test | Description |
| ---- | ---- |
| `test_cancel01.c` | Calling MPI_Cancel on an active request should cause an error; PASS = some sort of error; FAIL = unexpected timeout or incorrect/missing error. |
| `test_datatypeX.c` | Examples in chapter 4 of the specification use different combinations of sender and receiver contiguous datatypes as well as OpenMP pragmas. The datatype tests eschew the OpenMP and check just the combination of datatypes. |
| `test_datatype0.c` | Checks case where the same datatype is used on both sender and reciever, and the same number of partitions. PASS = run to completion. |
| `test_datatype1.c` | Based on example 4.3 in specification. This test has the sender and receiver use the same number of partitions. The sender uses a datatype (one per partition) while the receiver uses MPI_INT * PARTSIZE. PASS = run to completion. See also `test_datatype4.c` |
| `test_datatype2.c` | Based on example 4.3 in specification, this test has the sender use multiple partitions and the receiver use a single partition. The sender uses contiguous datatypes (one per partition) and the reciever MESSAGE_LENGTH. PASS = run to completion. See also `test_datatype4.c` |
| `test_datatype3.c` | Same as test_datatype2.c, except receiver uses a single contiguous datatype of size MESSAGE_LENGTH. See also `test_datatype5.c`, which does the same using Isend/Irecv. PASS = run to completion. |
| `test_datatype4.c` | A sanity check confirming the combination of sender-side contiguous datatype and receiver-side non-derived datatype (MPI_INT) works outside of partcomm with Isend/Irecv. PASS = run to completion |
| `test_datatype5.c` | A sanity check confirming the combination of multiple sender-side contiguous datatypes to a single receiver-side contiguous datatype works with Isend/Irecv. PASS = run to completion. |
| `test_example1a.c` | Example 1 from the 4.1 specification. PASS = run to completion |
| `test_example1b.c` | Example 1 from the 4.1 spec, except uses MPI_WAIT instead of MPI_TEST. PASS = run to completion |
| `test_example2.c` | Example 2 from the 4.1 specification. PASS = run to completion |
| `test_example3a.c` | Example 3 from the 4.1 specification, modified to not use any contiguous datatypes, use MPI_INT, and make the number of partitions the same on both sender and receiver. PASS = run to completion. |
| `test_example3b.c` | Example 3 from the 4.1 specification, modified to use MPI_INT and not use any contiguous datatypes. Uses multiple partitions on sender and one on receiver. |
| `test_example3c.c` | Example 3 from the 4.1 specification, modified to use MPI_INT. PASS = run to completion. See also `test_datatype[2,3].c` |
| `test_free0.c` | Calling MPI_Free on an active request should cause an error; PASS = some sort of error (TBD); FAIL = unexpected timeout or incorrect/missing error |
| `test_init0.c` | Tries to match a PSEND_INIT with an IRECV. PASS = expected timeout. |
| `test_init1.c` | Tries to match an ISEND with a PRECV_INIT. PASS = expected timeout. |
| `test_init2.c` | Tries to match a persistent send init (MPI_SEND_INIT) with a partcomm recv init (PRECV_INIT). PASS = expected timeout. |
| `test_local0.c` | Confirms partcomm calls are local, so the sender can get to test/wait before the receiver even calls PRECV_INIT PASS = runs to completion. |
| `test_local1.c` | Confirms partcomm calls are local, so the receiver can get to test/wait before the sender even calls PSEND_INIT. PASS = runs to completion |
| `test_numparts0.c` | Confirms a sender can have more partitions than a receiver. PASS = runs to completion. |
| `test_numparts1.c` | Confirms a receiver can have more partitions than a sender. PASS = runs to completion. |
| `test_order0.c` | Confirms the order in which partcomm init calls are made is the order they are matched (and hence filled). PASS = runs to completion; FAIL = likely fails check of received message contents |
| `test_parrived0.c` | Confirms PARRIVED works. Also (likely) calls PARRIVED more than once on the same partition. PASS = run to completion |
| `test_parrived1.c` | Confirms PARRIVED works on a request that has never been used (an inactive request). PASS = run to completion |
| `test_parrived2.c` | Tries to call PARRIVED on a request that does not correspond to a partitioned receive operation. PASS = some sort of error |
| `test_partitions0.c` | Tries to use PSEND_INIT with zero partitions. PASS = some sort of error |
| `test_partitions1.c` | Tries to use PSEND_INIT with negative partitions. PASS = some sort of error |
| `test_partitions2.c` | Tries to use PRECV_INIT with zero partitions. PASS = some sort of error |
| `test_partitions3.c` | Tries to use PRECV_INIT with negative partitions. PASS = some sort of error |
| `test_pready0.c` | Tries to call PREADY on a partition whose index is greater than the largest valid index. PASS = some sort of error |
| `test_pready1.c` | Tries to call PREADY on a partition whose index is equal to the number of partitions (so is 1 beyond the last valid index). PASS = some sort of error |
| `test_pready2.c` | Tries to call PREADY on a partition whose index is negative. PASS = some sort of error |
| `test_pready3.c` | Tries to call PREADY on a partition that has already been marked ready by PREADY. PASS = some sort of error |
| `test_pready4.c` | Tries to call PREADY on a request object that does not correspond to a partitioned send operation. PASS = some sort of error |
| `test_pready_range0.c` | Uses PREADY_RANGE in a standard way; PASS = successfully runs to completion |
| `test_pready_list0.c` | Uses PREADY_LIST in a standard way; PASS = runs to completion |
| `test_pready_list1.c` | Uses PREADY_LIST with the same partition appearing more than once. See also test_pready3.c. PASS = some sort of error |
| `test_startall0.c` | Sets up multiple PSEND_INIT + PRECV_INIT pairs using different tags and does partcomm for each. PASS = runs to completion. |
| `test_state0.c` | Confirms that the request status is reset when multiple rounds of START/PREADY/TEST are used. See also [OMPI Issue #12328](https://github.com/open-mpi/ompi/issues/12328) PASS = runs to completion. |
| `test_wildcard0.c` | Tries to use MPI_ANY_SOURCE with PRECV_INIT. PASS = an error of some sort |
| `test_wildcard0.c` | Tries to use MPI_ANY_SOURCE with PRECV_INIT. PASS = an error of some sort |
| `test_zerocount0.c` | Confirms PSEND_INIT and PRECV_INIT can use 0 partitions with a greater-than-zero count. PASS = runs to completion |
| `test_zerocount1.c` | Confirms PSEND_INIT and PRECV_INIT can use greater-than-zero partitions with a zero count. PASS = runs to completion |


