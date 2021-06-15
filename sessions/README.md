# Tests for MPI Sessions extensions to the MPI API

To build these tests you will need the following package:

- MPI Sessions prototype

## Installing the prototype

```
git clone --recursive git@github.com:hpc/ompi.git
cd ompi
git checkout sessions_new
./autogen.pl
./configure --prefix=my_sandbox 
make -j install
```

## Installing the tests

```
export PATH=my_sandbox/bin:$PATH
git clone https://github.com/open-mpi/ompi-tests-public.git
cd ompi-tests-public/sessions
make
```

## Running the tests

Assuming the checkout of the prototype occurred after 6/26/20, the tests can
be run using either the ```mpirun``` installed as part of the build of the
prototype, or prte/prun can be used.

When using prte, the tests can be run as follows:

```
prte --daemonize
prun -n 4 ./sessions_ex2 mpi://world
prun -n 4 ./sessions_ex3
prun -n 4 ./sessions_ex4
prun -n 4 ./sessions_test
```

To run using mpirun:

```
mpirun -np 4 ./sessions_ex2 mpi://world
mpirun -np 4 ./sessions_ex3
mpirun -np 4 ./sessions_ex4
mpirun -np 4 ./sessions_test
```

This example assumes your system has at least 4 slots available for MPI processes.

Note the third example may not have been built if you are using an old Fortran compiler 
that isn't able to generate the ```mpi_f08``` Fortran module.

## Special instructions for using prte/prun on a Cray XC and SLURM

On Cray XE/XC systems running SLURM, prte has to use a different procedure for launching
processes on the first node of a SLURM allocation.  Namely, prte launches a prte daemon
using the local slurmd daemon.  As a consequence, there are effectively two prte daemons
on the head node, the prte launched on the command line and a second launched via slurmd.

To avoid annoying things like loss of stdout/stdin from the processes launched on the head
node owing to having two prte daemons running on the node, additional arguments most be added to the prte and prun commands:

```
prte --daemonize --system-server
prun -n 4 --system-server-first (additional arguments)
```


# Old instructions

These instructions are applicable if you are working with a checkout
of the MPI Sessions prototype prior to 6/26/20.

To build these tests you will need the following several packages:

- most recent version of PMIx
- most recent version of prrte
- MPI Sessions prototype

## Installing PMIx

```
git clone git@github.com:pmix/pmix.git
cd pmix
./autogen.pl
./configure --prefix=my_sandbox --with-libevent
make -j install
```

## Installing prrte

```
git clone git@github.com:pmix/prrte.git
cd prrte
./autogen.pl
./configure --prefix=my_sandbox --with-libevent --with-pmix=my_sandbox
make -j install
```

## Installing the prototype

```
git clone git@github.com:hpc/ompi.git
cd ompi
git checkout sessions_new
./autogen.pl
./configure --prefix=my_sandbox --with-libevent --with-pmix=my_sandbox
make -j install
```

## Installing and running these tests

```
export PATH=my_sandbox/bin:$PATH
make
prte --daemonize
prun -n 4 ./sessions_ex2 mpi://world
prun -n 4 ./sessions_ex3
prun -n 4 ./sessions_ex4
prun -n 4 ./sessions_test
```
This example assumes your system has at least 4 slots available for MPI processes.

## Special instructions for Cray XC and SLURM

On Cray XE/XC systems running SLURM, prte has to use a different procedure for launching
processes on the first node of a SLURM allocation.  Namely, prte launches a prte daemon
using the local slurmd daemon.  As a consequence, there are effectively two prte daemons
on the head node, the prte launched on the command line and a second launched via slurmd.

To avoid annoying things like loss of stdout/stdin from the processes launched on the head
node owing to having two prte daemons running on the node, additional arguments most be added to the prte and prun commands:

```
prte --daemonize --system-server
prun -n 4 --system-server-first (additional arguments)
```

## Test documentation

sessions_test1: Initialize one session, finalize that sessions, then initialize another session

sessions_test2: Initialize two sessions and perform operations with each session simultaneously, then finalize both sessions

sessions_test3: Try to make a group from an invalid pset (should fail)

sessions_test4: Try to make a group from a pset using MPI_GROUP_NULL (should fail)

sessions_test5: Initialize two sessions, perform functions with one session and finalize that session, then perform functions with the other session after the first has been finalized

sessions_test6: Same as sessions_test1 but with the sessions using different names

sessions_test7: Initialize two sessions, perform operations with one and finalize it, then perform operations with the other and finalize it

sessions_test8: Initialize two sessions, create one comm in each, and compare them (should fail because objects from different sessions shall not be intermixed with each other in a single MPI procedure call per the MPI standard)

sessions_test9: Initialize the World model and Sessions model, make a comm using the sessions and split it, then make an intercomm using the split comm from the session and MPI_COMM_WORLD (should fail because MPI objects derived from the Sessions model shall not be intermixed in a single MPI
procedure call with MPI objects derived from the World model per the MPI standard)

sessions_test10: Initialize World model, initialize Sessions model, finalize Sessions model, finalize World model

sessions_test11: Initialize World model, initialize Sessions model, finalize World model, finalize Sessions model

sessions_test12: Initialize Sessions model, initialize World model, finalize Sessions model, finalize World model

sessions_test13: Initialize Sessions model, initialize World model, finalize World model, finalize Sessions model

sessions_test14: Initialize a session, create a comm, then try to use that comm with a comm from MPI_Comm_get_parent (should fail because MPI objects derived from the Sessions Model shall not be intermixed in a single MPI procedure call with MPI objects derived from the communicator obtained from a call to MPI_COMM_GET_PARENT or MPI_COMM_JOIN)

sessions_test15: Initialize two sessions, create MPI_Requests using comms from different sessions, then include MPI_Requests from different sessions in one call to each of the following functions: MPI_Waitall(), MPI_Waitsome(), MPI_Waitany(), MPI_Testall(), MPI_Testsome(), and MPI_Testany()

sessions_test16: Initialize a sessions, create a comm from the session, then attempt to access the values of the default attributes usually attached to MPI_COMM_WORLD (MPI_TAG_UB, MPI_HOST, MPI_IO, and MPI_WTIME_IS_GLOBAL). Per the MPI Standard, only the MPI_TAG_UB attribute should be accessible when using the Sessions model.


