# Test MPI_Status conversion functions

MPI-4:18.2.5 has a good diagram showing all the possible conversion
functions for an MPI_Status.  We need to test each edge in that
diagram.

```
                     MPI_Status
                       / ^ \ ^
  C types and         / /   \ \
  functions          / /     \ \
                 (1)/ /(2) (3)\ \(4)
                   / /   (5)   \ \
                  / / <-------- ' \
 MPI_F08_status  ' --------------> \ MPI_Fint array
                         (6)

                         (7)
 TYPE(MPI_Status)  <-------------- INTEGER array of size
                   --------------> MPI_STATUS_SIZE (in Fortran)
		         (8)

```

1. MPI_Status_c2f08()
1. MPI_Status_f082c()
1. MPI_Status_c2f()
1. MPI_Status_f2c()
1. MPI_Status_f2f08() (in C)
1. MPI_Status_f082f() (in C)
1. MPI_Status_f2f08() (in Fortran)
   1. In the `mpi` module
   1. In the `mpi_f08` module
1. MPI_Status_f082f() (in Fortran)
   1. In the `mpi` module
   1. In the `mpi_f08` module

By transitive property, if we test each leg in the above diagram, then
we effectively certify the conversion journey of a status across any
combination of the legs in the diagram (i.e., any possible correct
status conversion).
