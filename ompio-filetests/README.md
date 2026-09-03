# MPI File I/O Tests

Tests for MPI-I/O operations (open, read, write, fileview, atomicity, shared pointers, non-blocking I/O, etc.).

## Building

Tests are built in a separate build directory and run from this source directory.

### Prerequisites

- CMake 3.10+
- An MPI installation (e.g. Open MPI)

### Configure and build

```sh
cmake -S . -B build -DMPI_HOME=/path/to/mpi
cmake --build build
```

If `MPI_HOME` is not specified, CMake will search for MPI in standard system paths.

### Build modes

#### Combined executable (default)

All tests compiled into a single `filetest` binary (equivalent to `make filetest`):

```sh
cmake -S . -B build -DMPI_HOME=/path/to/mpi -DBUILD_COMBINED=ON
cmake --build build
```

#### Individual executables

Each test compiled into its own binary, which simplifies debugging:

```sh
cmake -S . -B build -DMPI_HOME=/path/to/mpi -DBUILD_COMBINED=OFF
cmake --build build
```

To build a single target:

```sh
cmake --build build --target write
```

Available individual targets: `open`, `cart_open`, `write`, `iwrite`, `read`, `iread`,
`fileview`, `write_all`, `read_all`, `write_shared`, `read_shared`, `info`, `atomicity`,
`utils`, `mt_open`, `mt_write`.

## Running

The combined `filetest` binary requires exactly 6 MPI processes:

```sh
mpirun -np 6 build/filetest
```

Individual tests may have different process count requirements; check the source for details.
