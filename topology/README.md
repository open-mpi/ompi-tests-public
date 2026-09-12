# Topology tests

Two standalone tests for the virtual topology constructors and the
neighborhood collectives.  Plain MPI, no dependencies beyond an MPI
implementation, and neither reads a stored reference file: both check what
the standard requires, so they cannot drift with the implementation they
exercise.

```
make
```

Both return a non-zero exit status if any case fails.

## neighbor_alltoall_conformance

Checks `MPI_Neighbor_alltoall`, `MPI_Neighbor_alltoallv` and
`MPI_Neighbor_alltoallw`, each in blocking, nonblocking and persistent form,
on a periodic 3-D Cartesian communicator and on a distributed graph built
over the same neighbour list.

The two topology families pair duplicate neighbour edges differently, and
MPI-4.1 Section 8.6 defines both:

* **Cartesian.** Block `s` of the sender lands in block `s^1` of the
  receiver, in every dimension, degenerate or not.  The Advice to
  implementors there names the `periods[d] == 1 && dims[d] == 1 or 2` case
  explicitly, and MPI-4.0 Annex B.1.1 item 1 applies the rule to MPI-3.1 as
  errata.
* **Graph and distributed graph.** The k-th edge to a process matches the
  k-th edge from it.

```
mpirun -n 2  ./neighbor_alltoall_conformance 1 1 2
mpirun -n 18 ./neighbor_alltoall_conformance 2 3 3
mpirun -n 27 ./neighbor_alltoall_conformance 3 3 3
```

The three arguments are the process grid; omit them and `MPI_Dims_create`
chooses.  The shape matters: where every `dims[d] >= 3` the two neighbours
along each direction are distinct processes, rank matching alone forces the
pairing, and the test cannot discriminate.  A periodic dimension of size 1 or
2 makes both neighbours the same process, which is the case worth running.
`3 3 3` is therefore a control rather than a test.

Set `NBR_CART_ONLY=1` to skip the distributed graph pass.

## dist_graph_constructors

Twelve cases covering `MPI_Dist_graph_create` and
`MPI_Dist_graph_create_adjacent` built from parents that do and do not carry
a topology of their own.  Cached information propagates only through
duplication, so a distributed graph must report `MPI_DIST_GRAPH`, must refuse
`MPI_Cartdim_get`, and must deliver neighborhood alltoall per Section 8.6,
whatever its parent was.

```
mpirun -n 4 ./dist_graph_constructors adj-cart
```

One case per run, named by the single argument:

| case | |
|---|---|
| `adj-world` | adjacent, from `MPI_COMM_WORLD` |
| `adj-cart`, `gen-cart` | from a Cartesian communicator |
| `adj-graph`, `gen-graph` | from an `MPI_Graph` communicator |
| `adj-cart-info`, `gen-cart-info` | as above, with a non-null `MPI_Info` |
| `adj-cart-reorder`, `gen-cart-reorder` | as above, with `reorder` set |
| `adj-distgraph` | from another distributed graph |
| `dup-of-result` | `MPI_Comm_dup` of the resulting graph |
| `stress` | 3 x 2000 create/free cycles, then the parents are re-checked |

Worth running under `--mca topo basic` as well, since a Cartesian parent and
the general constructor otherwise take different paths.

## Licence

Copyright (c) 2026 Metere Consulting, LLC.  Distributed under the
BSD-3-Clause terms in the LICENSE file at the root of this repository.
