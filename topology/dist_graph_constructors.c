/*
 * Copyright (c) 2026      Metere Consulting, LLC.  All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * dist_graph_constructors.c
 *
 * Distributed graph communicators created from parents that carry a
 * topology.  One case per run (argv[1]); rank 0 prints PASS or FAIL.
 *
 * Every created graph must report MPI_DIST_GRAPH, refuse MPI_Cartdim_get,
 * and deliver neighborhood alltoall (blocking and nonblocking) per MPI-4.1
 * section 8.6: the k-th edge from a process matches the k-th edge to it.
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXD 8
static char why[512];

static void note(const char *s) { strncat(why, s, sizeof(why) - strlen(why) - 1); }

static int verify(MPI_Comm g)
{
    int st = -1, rc, in = 0, out = 0, w = 0, nd = -1, ok = 1, grank, gsize;
    char b[128];
    MPI_Comm_set_errhandler(g, MPI_ERRORS_RETURN);
    MPI_Comm_rank(g, &grank);
    MPI_Comm_size(g, &gsize);
    MPI_Topo_test(g, &st);
    if (st != MPI_DIST_GRAPH) { snprintf(b, sizeof b, " Topo_test!=DIST_GRAPH(%d)", st); note(b); ok = 0; }
    if (MPI_Cartdim_get(g, &nd) == MPI_SUCCESS) { snprintf(b, sizeof b, " Cartdim_get succeeded(%d)", nd); note(b); ok = 0; }
    MPI_Dist_graph_neighbors_count(g, &in, &out, &w);
    if (in > MAXD || out > MAXD) { note(" degree too large"); return 0; }
    int srcs[MAXD], dsts[MAXD], sw[MAXD], dw[MAXD], mine[MAXD], exp[MAXD], sb[MAXD], rb[MAXD];
    MPI_Dist_graph_neighbors(g, in, srcs, w ? sw : MPI_UNWEIGHTED, out, dsts, w ? dw : MPI_UNWEIGHTED);
    int *alld = malloc(sizeof(int) * MAXD * gsize);
    for (int i = 0; i < MAXD; i++) mine[i] = i < out ? dsts[i] : -1;
    MPI_Allgather(mine, MAXD, MPI_INT, alld, MAXD, MPI_INT, g);
    for (int l = 0; l < in; l++) {
        int P = srcs[l], m = 0;
        exp[l] = -1;
        for (int k = 0; k < l; k++) m += srcs[k] == P;
        for (int j = 0; j < MAXD; j++)
            if (alld[P * MAXD + j] == grank && m-- == 0) { exp[l] = j; break; }
    }
    free(alld);
    for (int nb = 0; nb < 2; nb++) {
        MPI_Request req;
        for (int k = 0; k < out; k++) sb[k] = grank * 100 + k;
        for (int l = 0; l < in; l++) rb[l] = -1;
        rc = nb ? MPI_Ineighbor_alltoall(sb, 1, MPI_INT, rb, 1, MPI_INT, g, &req)
                : MPI_Neighbor_alltoall(sb, 1, MPI_INT, rb, 1, MPI_INT, g);
        if (nb && rc == MPI_SUCCESS) rc = MPI_Wait(&req, MPI_STATUS_IGNORE);
        if (rc != MPI_SUCCESS) { note(nb ? " Ineighbor_alltoall error" : " Neighbor_alltoall error"); ok = 0; continue; }
        for (int l = 0; l < in; l++)
            if (exp[l] < 0 || rb[l] != srcs[l] * 100 + exp[l]) { note(nb ? " nonblocking data wrong" : " blocking data wrong"); ok = 0; break; }
    }
    return ok;
}

/* ring neighbours in the parent's rank numbering */
static void ring(MPI_Comm parent, int nb[2])
{
    int r, s;
    MPI_Comm_rank(parent, &r); MPI_Comm_size(parent, &s);
    nb[0] = (r + s - 1) % s; nb[1] = (r + 1) % s;
}

static MPI_Comm adj(MPI_Comm parent, MPI_Info info, int reorder)
{
    int nb[2]; MPI_Comm g;
    ring(parent, nb);
    MPI_Dist_graph_create_adjacent(parent, 2, nb, MPI_UNWEIGHTED, 2, nb, MPI_UNWEIGHTED, info, reorder, &g);
    return g;
}

static MPI_Comm gen(MPI_Comm parent, MPI_Info info, int reorder)
{
    int nb[2], me, deg = 2; MPI_Comm g;
    ring(parent, nb); MPI_Comm_rank(parent, &me);
    MPI_Dist_graph_create(parent, 1, &me, &deg, nb, MPI_UNWEIGHTED, info, reorder, &g);
    return g;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size, dims[3] = {0, 0, 0}, periods[3] = {1, 1, 1};
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
    MPI_Dims_create(size, 3, dims);

    MPI_Comm cart, graph, dgp, g = MPI_COMM_NULL;
    MPI_Cart_create(MPI_COMM_WORLD, 3, dims, periods, 0, &cart);
    MPI_Comm_set_errhandler(cart, MPI_ERRORS_RETURN);
    int *index = malloc(sizeof(int) * size), *edges = malloc(sizeof(int) * 2 * size);
    for (int i = 0; i < size; i++) {
        index[i] = 2 * (i + 1); edges[2 * i] = (i + size - 1) % size; edges[2 * i + 1] = (i + 1) % size;
    }
    MPI_Graph_create(MPI_COMM_WORLD, size, index, edges, 0, &graph);
    MPI_Comm_set_errhandler(graph, MPI_ERRORS_RETURN);
    {   /* a dist-graph parent with a different edge set: only the right neighbour */
        int l = (rank + size - 1) % size, r = (rank + 1) % size;
        MPI_Dist_graph_create_adjacent(MPI_COMM_WORLD, 1, &l, MPI_UNWEIGHTED, 1, &r, MPI_UNWEIGHTED,
                                       MPI_INFO_NULL, 0, &dgp);
        MPI_Comm_set_errhandler(dgp, MPI_ERRORS_RETURN);
    }
    MPI_Info info;
    MPI_Info_create(&info);
    MPI_Info_set(info, "mpi_assert_no_any_tag", "true");

    const char *c = argv[1];
    int ok = 1;
    why[0] = 0;
    if      (!strcmp(c, "adj-world"))        ok = verify(g = adj(MPI_COMM_WORLD, MPI_INFO_NULL, 0));
    else if (!strcmp(c, "adj-cart"))         ok = verify(g = adj(cart, MPI_INFO_NULL, 0));
    else if (!strcmp(c, "gen-cart"))         ok = verify(g = gen(cart, MPI_INFO_NULL, 0));
    else if (!strcmp(c, "adj-graph"))        ok = verify(g = adj(graph, MPI_INFO_NULL, 0));
    else if (!strcmp(c, "gen-graph"))        ok = verify(g = gen(graph, MPI_INFO_NULL, 0));
    else if (!strcmp(c, "adj-cart-info"))    ok = verify(g = adj(cart, info, 0));
    else if (!strcmp(c, "gen-cart-info"))    ok = verify(g = gen(cart, info, 0));
    else if (!strcmp(c, "adj-cart-reorder")) ok = verify(g = adj(cart, MPI_INFO_NULL, 1));
    else if (!strcmp(c, "gen-cart-reorder")) ok = verify(g = gen(cart, MPI_INFO_NULL, 1));
    else if (!strcmp(c, "adj-distgraph")) {
        int in, out, w;
        ok = verify(g = adj(dgp, MPI_INFO_NULL, 0));
        MPI_Dist_graph_neighbors_count(g, &in, &out, &w);
        if (in != 2 || out != 2) { note(" child reports parent's degrees"); ok = 0; }
        MPI_Dist_graph_neighbors_count(dgp, &in, &out, &w);
        if (in != 1 || out != 1) { note(" parent degrees changed"); ok = 0; }
    } else if (!strcmp(c, "dup-of-result")) {
        MPI_Comm d, g0 = adj(cart, MPI_INFO_NULL, 0);
        MPI_Comm_dup(g0, &d);
        ok = verify(d);
        MPI_Comm_free(&d);
        g = g0;
    } else if (!strcmp(c, "stress")) {
        for (int i = 0; i < 2000; i++) {
            MPI_Comm t = adj(cart, MPI_INFO_NULL, 0); MPI_Comm_free(&t);
            t = gen(cart, MPI_INFO_NULL, 0);          MPI_Comm_free(&t);
            t = adj(graph, MPI_INFO_NULL, 0);         MPI_Comm_free(&t);
        }
        int st, nd, s0, d0;
        MPI_Topo_test(cart, &st);
        MPI_Cartdim_get(cart, &nd);
        if (st != MPI_CART || nd != 3) { note(" Cartesian parent damaged"); ok = 0; }
        if (MPI_Cart_shift(cart, 0, 1, &s0, &d0) != MPI_SUCCESS) { note(" Cart_shift on parent failed"); ok = 0; }
        MPI_Topo_test(graph, &st);
        if (st != MPI_GRAPH) { note(" graph parent damaged"); ok = 0; }
        ok &= verify(g = adj(cart, MPI_INFO_NULL, 0));
    } else {
        if (rank == 0) printf("FAIL %s: unknown case\n", c);
        MPI_Finalize();
        return 2;
    }

    int all;
    MPI_Allreduce(&ok, &all, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
    if (rank == 0) printf("%s %s%s%s\n", all ? "PASS" : "FAIL", c, all ? "" : ":", all ? "" : why);
    if (g != MPI_COMM_NULL) MPI_Comm_free(&g);
    MPI_Info_free(&info);
    MPI_Comm_free(&dgp); MPI_Comm_free(&graph); MPI_Comm_free(&cart);
    free(index); free(edges);
    MPI_Finalize();
    return all ? 0 : 1;
}
