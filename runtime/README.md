# Test suite for Open MPI runtime

This test suite is meant to be able to be run stand-alone or under CI.

All of the tests that are intended for CI must be listed in the  `.ci-tests` file.

If the Open MPI build needs additional `configure` options those can be added to the `.ci-configure` file.

## Running tests stand alone

 1. Make sure that Open MPI and other required libraries are in your `PATH`/`LD_LIBRARY_PATH`
 2. Drop into a directory:
    - Use the `build.sh` script to build any test articles
    - Use the `run.sh` script to run the test program


## CI Environment Variables

The CI infrastructure defines the following environment variables to be used in the test programs. These are defined during the `run.sh` phase and not the `build.sh` phase.

 * `CI_HOSTFILE` : Absolute path to the hostfile for this run.
 * `CI_NUM_NODES` : Number of nodes in this cluster.
 * `CI_OMPI_SRC` : top level directory of the Open MPI repository checkout.
 * `CI_OMPI_TESTS_PUBLIC_DIR` : Top level directory of the [Open MPI Public Test](https://github.com/open-mpi/ompi-tests-public) repository checkout
 * `OMPI_ROOT` : Open MPI install directory.


### Adding a new test for CI

 1. Create a directory with your test.
    - **Note**: Please make your test scripts such that they can be easily run with or without the CI environment variables.
 2. Create a build script named `build.sh`
    - CI will call this exactly one time (with a timeout in case it hangs).
    - If the script returns `0` then it is considered successful. Otherwise it is considered failed.
 3. Create a run script named `run.sh`
    - The script is responsible for running your test including any runtime setup/shutdown and test result inspection.
    - CI will call this exactly one time (with a timeout in case it hangs).
    - If the script returns `0` then it is considered successful. Otherwise it is considered failed.
 4. Add your directory name to the `.ci-tests` file in this directory in the order that they should be executed.
    - Note that adding the directory is not sufficient to have CI run the test, it must be in the file.
    - Comments (starting with `#`) are allowed.
