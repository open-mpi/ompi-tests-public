/*
 * Copyright (c) 2026      Metere Consulting, LLC.  All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * neighbor_alltoall_conformance.c
 *
 * Checks MPI_Neighbor_alltoall{,v,w} in blocking, nonblocking and persistent
 * form on a periodic 3-D Cartesian communicator. MPI-4.x section 8.6
 * (Example 8.10) requires receive block l to hold the block that the
 * neighbour in direction l sent towards this process, i.e. that neighbour's
 * send block l^1. This holds in every dimension, including dims[d] == 1 or 2
 * where the two neighbours along d are the same process.
 *
 * The same neighbour list is then used to build an MPI_Dist_graph, where
 * section 8.6 instead requires the k-th edge to a process to match the k-th
 * edge from it (list order), to check that graph topologies are unaffected.
 *
 * Usage: mpirun -n P ./nbr_coll_conformance d0 d1 d2   (d0*d1*d2 == P)
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define NDIM 3
#define NNBR (2 * NDIM)
#define N 4                     /* ints per block */

enum { A2A, A2AV, A2AW, NCOLL };
enum { BLOCKING, NONBLOCKING, PERSISTENT, NMODE };
static const char *cname[NCOLL] = { "alltoall ", "alltoallv", "alltoallw" };
static const char *mname[NMODE] = { "blocking", "nonblocking", "persistent" };

static int enc(int rank, int blk, int j) { return rank * 10000 + blk * 100 + j; }

static long run(MPI_Comm cart, int coll, int mode, const int *nbr, const int *exp, int rank)
{
    int sbuf[NNBR * N], rbuf[NNBR * N], cnt[NNBR], dsp[NNBR];
    MPI_Aint bdsp[NNBR];
    MPI_Datatype ty[NNBR];
    MPI_Request req = MPI_REQUEST_NULL;
    int rc = MPI_SUCCESS;

    for (int k = 0; k < NNBR; k++) {
        for (int j = 0; j < N; j++) {
            sbuf[k * N + j] = enc(rank, k, j);
            rbuf[k * N + j] = -1;
        }
        cnt[k] = N;
        dsp[k] = k * N;
        bdsp[k] = (MPI_Aint) k * N * (MPI_Aint) sizeof(int);
        ty[k] = MPI_INT;
    }

    switch (mode) {
    case BLOCKING:
        if (coll == A2A)
            rc = MPI_Neighbor_alltoall(sbuf, N, MPI_INT, rbuf, N, MPI_INT, cart);
        else if (coll == A2AV)
            rc = MPI_Neighbor_alltoallv(sbuf, cnt, dsp, MPI_INT, rbuf, cnt, dsp, MPI_INT, cart);
        else
            rc = MPI_Neighbor_alltoallw(sbuf, cnt, bdsp, ty, rbuf, cnt, bdsp, ty, cart);
        break;
    case NONBLOCKING:
        if (coll == A2A)
            rc = MPI_Ineighbor_alltoall(sbuf, N, MPI_INT, rbuf, N, MPI_INT, cart, &req);
        else if (coll == A2AV)
            rc = MPI_Ineighbor_alltoallv(sbuf, cnt, dsp, MPI_INT, rbuf, cnt, dsp, MPI_INT, cart, &req);
        else
            rc = MPI_Ineighbor_alltoallw(sbuf, cnt, bdsp, ty, rbuf, cnt, bdsp, ty, cart, &req);
        if (rc == MPI_SUCCESS)
            rc = MPI_Wait(&req, MPI_STATUS_IGNORE);
        break;
    case PERSISTENT:
#if MPI_VERSION >= 4
        if (coll == A2A)
            rc = MPI_Neighbor_alltoall_init(sbuf, N, MPI_INT, rbuf, N, MPI_INT, cart,
                                       MPI_INFO_NULL, &req);
        else if (coll == A2AV)
            rc = MPI_Neighbor_alltoallv_init(sbuf, cnt, dsp, MPI_INT, rbuf, cnt, dsp, MPI_INT,
                                        cart, MPI_INFO_NULL, &req);
        else
            rc = MPI_Neighbor_alltoallw_init(sbuf, cnt, bdsp, ty, rbuf, cnt, bdsp, ty,
                                        cart, MPI_INFO_NULL, &req);
        if (rc == MPI_SUCCESS)
            rc = MPI_Start(&req);
        if (rc == MPI_SUCCESS)
            rc = MPI_Wait(&req, MPI_STATUS_IGNORE);
        if (req != MPI_REQUEST_NULL)
            MPI_Request_free(&req);
#else
        return -1;              /* not available before MPI-4 */
#endif
        break;
    }

    if (rc != MPI_SUCCESS)
        return -2;              /* the library returned an error */

    long bad = 0;
    for (int l = 0; l < NNBR; l++) {
        if (nbr[l] == MPI_PROC_NULL)
            continue;
        for (int j = 0; j < N; j++)
            if (rbuf[l * N + j] != enc(nbr[l], exp[l], j))
                bad++;
    }
    return bad;
}

int main(int argc, char **argv)
{
    int size, rank, dims[NDIM] = { 0, 0, 0 }, periods[NDIM] = { 1, 1, 1 }, nbr[NNBR];
    MPI_Comm cart;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (argc > NDIM)
        for (int d = 0; d < NDIM; d++)
            dims[d] = atoi(argv[d + 1]);
    MPI_Dims_create(size, NDIM, dims);
    MPI_Cart_create(MPI_COMM_WORLD, NDIM, dims, periods, 0, &cart);
    MPI_Comm_set_errhandler(cart, MPI_ERRORS_RETURN);
    MPI_Comm_rank(cart, &rank);
    for (int d = 0; d < NDIM; d++)
        MPI_Cart_shift(cart, d, 1, &nbr[2 * d], &nbr[2 * d + 1]);

    int fail = 0;
    if (rank == 0) {
        char v[MPI_MAX_LIBRARY_VERSION_STRING];
        int len;
        MPI_Get_library_version(v, &len);
        for (char *p = v; *p; p++)
            if (*p == '\n') { *p = 0; break; }
        printf("%s\ndims %d x %d x %d, periodic\n", v, dims[0], dims[1], dims[2]);
    }
    /* Cartesian: slot l receives the neighbour's block l^1.  Graph: slot l,
     * the m-th edge from peer P, receives the block P sent on its m-th edge
     * to this process. */
    int expc[NNBR], expg[NNBR], *alln = malloc(sizeof(int) * NNBR * size);
    MPI_Allgather(nbr, NNBR, MPI_INT, alln, NNBR, MPI_INT, cart);
    for (int l = 0; l < NNBR; l++) {
        int P = nbr[l], m = 0;
        expc[l] = l ^ 1;
        expg[l] = -1;
        if (P == MPI_PROC_NULL)
            continue;
        for (int k = 0; k < l; k++)
            m += nbr[k] == P;
        for (int j = 0; j < NNBR; j++)
            if (alln[P * NNBR + j] == rank && m-- == 0) {
                expg[l] = j;
                break;
            }
    }
    free(alln);
    MPI_Comm graph;
    int *unw = MPI_UNWEIGHTED;    /* sentinel pointer; never dereferenced */
    MPI_Dist_graph_create_adjacent(cart, NNBR, nbr, unw, NNBR, nbr, unw,
                                   MPI_INFO_NULL, 0, &graph);
    MPI_Comm_set_errhandler(graph, MPI_ERRORS_RETURN);

    /* NBR_CART_ONLY=1 skips the dist-graph pass, e.g. on a library that
     * crashes on repeated graph edges before reporting anything. */
    const char *co = getenv("NBR_CART_ONLY");
    const int ntopo = (co && *co == '1') ? 1 : 2;
    for (int t = 0; t < ntopo; t++) {
        MPI_Comm comm = t ? graph : cart;
        if (rank == 0)
            printf(" %s\n", t ? "dist-graph, same neighbour list (list order required)"
                              : "Cartesian (block s -> block s^1 required)");
        for (int c = 0; c < NCOLL; c++)
            for (int m = 0; m < NMODE; m++) {
                long bad = run(comm, c, m, nbr, t ? expg : expc, rank), tot = 0;
                long st = bad < 0 ? bad : 0, cnt = bad < 0 ? 0 : bad, minst = 0;
                MPI_Reduce(&cnt, &tot, 1, MPI_LONG, MPI_SUM, 0, cart);
                MPI_Reduce(&st, &minst, 1, MPI_LONG, MPI_MIN, 0, cart);
                if (rank == 0) {
                    if (minst == -1)
                        printf("  %s %-11s : n/a\n", cname[c], mname[m]);
                    else if (minst == -2) {
                        printf("  %s %-11s : ERROR (library returned an error)\n", cname[c], mname[m]);
                        fail = 1;
                    } else {
                        printf("  %s %-11s : %s (%ld wrong)\n", cname[c], mname[m],
                               tot ? "FAIL" : "pass", tot);
                        fail |= tot != 0;
                    }
                }
            }
    }
    MPI_Comm_free(&graph);
    MPI_Bcast(&fail, 1, MPI_INT, 0, cart);
    MPI_Comm_free(&cart);
    MPI_Finalize();
    return fail;
}
