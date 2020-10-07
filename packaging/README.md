# Test for exported symbol name pollution

This runs from inside a rank and uses ldd on itself to figure out what
libraries to examine (it looks for a libmpi.so in the ldd output, and
then decides that any other libraries that come from the same directory
as libmpi.so must also be part of OMPI and should be examined too), then
nm on those MPI libraries to look for symbols without some accepted
OMPI prefix.

Example runs with good/bad output::

```
% export OPAL_PREFIX=/some/path
% export OLDPATH=$PATH
% export PATH=$OPAL_PREFIX/bin:${PATH}
% export LD_LIBRARY_PATH=$OPAL_PREFIX/lib
% autoreconf -i
% make

------------------------------------------------
*** Example of a passing run:

% $OPAL_PREFIX/bin/mpirun -np 1 ./run_nmcheck

> Checking for bad symbol names:
> *** checking /some/path/lib/libpmix.so.0
> *** checking /some/path/lib/libopen-pal.so.0
> *** checking /some/path/lib/libmpi.so.0
> *** checking /some/path/lib/libmpi_mpifh.so
> *** checking /some/path/lib/libmpi_usempif08.so
> *** checking /some/path/lib/libmpi_usempi_ignore_tkr.so

------------------------------------------------
*** Example of a failing run:

Then if I edit one of the opal C files to add a couple
extraneous globally exported symbols
    int myfunction() { return 0; }
    int myglobal = 123;
and rerun the test:

% $OPAL_PREFIX/bin/mpirun -np 1 ./run_nmcheck

> Checking for bad symbol names:
> *** checking /u/markalle/space/Projects/OMPIDev.m/install/lib/libmpi.so.0
> *** checking /u/markalle/space/Projects/OMPIDev.m/install/lib/libpmix.so.0
> *** checking /u/markalle/space/Projects/OMPIDev.m/install/lib/libopen-pal.so.0
>     [error]   myfunction
>     [error]   myglobal
> *** checking /u/markalle/space/Projects/OMPIDev.m/install/lib/libmpi_mpifh.so
> *** checking /u/markalle/space/Projects/OMPIDev.m/install/lib/libmpi_usempif08.so
> *** checking /u/markalle/space/Projects/OMPIDev.m/install/lib/libmpi_usempi_ignore_tkr.so
> --------------------------------------------------------------------------
> MPI_ABORT was invoked on rank 0 in communicator MPI_COMM_WORLD
>   Proc: [[27901,1],0]
>   Errorcode: 16
>
> NOTE: invoking MPI_ABORT causes Open MPI to kill all MPI processes.
> You may or may not see output from other processes, depending on
> exactly when Open MPI kills them.
> --------------------------------------------------------------------------
```
