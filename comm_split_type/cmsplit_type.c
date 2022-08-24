/*
 * Modified / Simplified version of MPICH MPI test suite version
 * for Open MPI specific split_topo values:
 *  mpich-testsuite-4.1a1/comm/cmsplit_type.c
 *
 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static const char *split_topo[] = {
                                   "mpi_shared_memory",
                                   "hwthread",
                                   "core",
                                   "l1cache",
                                   "l2cache",
                                   "l3cache",
                                   "socket",
                                   "numanode",
                                   "board",
                                   "host",
                                   "cu",
                                   "cluster",
                                   NULL
};

int verbose = 0;
int mcw_rank = 0;
int mcw_size = 0;

static void sync_hr(void) {
    if (verbose) {
        fflush(NULL);
        MPI_Barrier(MPI_COMM_WORLD);
        if (mcw_rank == 0) {
            printf("-----------------------------------\n");
            usleep(1000000);
        }
        fflush(NULL);
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[])
{
    int rank, size, errs = 0, tot_errs = 0;
    int i;
    MPI_Comm comm;
    MPI_Info info;
    int ret;
    int value = 0;
    int expected_value = 3;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mcw_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mcw_size);

    /*
     * Verbosity
     */
    if (getenv("MPITEST_VERBOSE")) {
        verbose = 1;
    }
    MPI_Bcast(&verbose, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /*
     * Check to see if MPI_COMM_TYPE_SHARED works correctly
     */
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &comm);
    if (comm == MPI_COMM_NULL) {
        printf("MPI_COMM_TYPE_SHARED (no hint): got MPI_COMM_NULL\n");
        errs++;
    } else {
        MPI_Comm_rank(comm, &rank);
        MPI_Comm_size(comm, &size);
        if (rank == 0 && verbose) {
            printf("MPI_COMM_TYPE_SHARED (no hint): Created shared subcommunicator of size %d\n",
                   size);
        }
        MPI_Comm_free(&comm);
    }

    sync_hr();

    /*
     * Test MPI_COMM_TYPE_HW_GUIDED:
     *  - Test with "mpi_hw_resource_type" = "mpi_shared_memory"
     */
    MPI_Info_create(&info);
    MPI_Info_set(info, "mpi_hw_resource_type", "mpi_shared_memory");
    if (mcw_rank == 0 && verbose) {
        printf("MPI_COMM_TYPE_HW_GUIDED: Trying MPI Standard value %s\n", "mpi_shared_memory");
    }
    ret = MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_HW_GUIDED, 0, info, &comm);
    if (ret != MPI_SUCCESS) {
        printf("MPI_COMM_TYPE_HW_GUIDED (%s) failed\n", split_topo[i]);
        errs++;
    } else if (comm != MPI_COMM_NULL) {
        MPI_Comm_rank(comm, &rank);
        MPI_Comm_size(comm, &size);
        if (rank == 0 && verbose) {
            printf("MPI_COMM_TYPE_HW_GUIDED (%s): Created shared subcommunicator of size %d\n",
                   "mpi_shared_memory", size);
        }
        MPI_Comm_free(&comm);
    }
    MPI_Info_free(&info);

    sync_hr();

    /*
     * Test MPI_COMM_TYPE_HW_GUIDED:
     *  - Test with a variety of supported info values
     */
    for (i = 0; split_topo[i]; i++) {
        MPI_Info_create(&info);
        MPI_Info_set(info, "mpi_hw_resource_type", split_topo[i]);
        if (mcw_rank == 0 && verbose) {
            printf("MPI_COMM_TYPE_HW_GUIDED: Trying value %s\n", split_topo[i]);
        }
        ret = MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_HW_GUIDED, 0, info, &comm);
        /* result will depend on platform and process bindings, just check returns */
        if (ret != MPI_SUCCESS) {
            printf("MPI_COMM_TYPE_HW_GUIDED (%s) failed\n", split_topo[i]);
            errs++;
        } else if (comm != MPI_COMM_NULL) {
            MPI_Comm_rank(comm, &rank);
            MPI_Comm_size(comm, &size);
            if (rank == 0 && verbose) {
                printf("MPI_COMM_TYPE_HW_GUIDED (%s): Created shared subcommunicator of size %d\n",
                       split_topo[i], size);
            }
            MPI_Comm_free(&comm);
        }
        MPI_Info_free(&info);
        sync_hr();
    }

    /*
     * Test MPI_COMM_TYPE_HW_GUIDED:
     *  - pass MPI_INFO_NULL, it must return MPI_COMM_NULL.
     */
    if (mcw_rank == 0 && verbose) {
        printf("MPI_COMM_TYPE_HW_GUIDED: Trying MPI_INFO_NULL\n");
    }
    info = MPI_INFO_NULL;
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_HW_GUIDED, 0, info, &comm);
    if (comm != MPI_COMM_NULL) {
        printf("MPI_COMM_TYPE_HW_GUIDED with MPI_INFO_NULL didn't return MPI_COMM_NULL\n");
        errs++;
        MPI_Comm_free(&comm);
    } else {
        if (mcw_rank == 0 && verbose) {
            printf("MPI_COMM_TYPE_HW_GUIDED: Trying MPI_INFO_NULL - Passed\n");
        }
    }

    sync_hr();

    /*
     * Test MPI_COMM_TYPE_HW_GUIDED:
     *  - info without correct key, it must return MPI_COMM_NULL.
     */
    if (mcw_rank == 0 && verbose) {
        printf("MPI_COMM_TYPE_HW_GUIDED: Trying invalid key\n");
    }
    MPI_Info_create(&info);
    MPI_Info_set(info, "bogus_key", split_topo[0]);
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_HW_GUIDED, 0, info, &comm);
    if (comm != MPI_COMM_NULL) {
        printf("MPI_COMM_TYPE_HW_GUIDED without correct key didn't return MPI_COMM_NULL\n");
        errs++;
        MPI_Comm_free(&comm);
    } else {
        if (mcw_rank == 0 && verbose) {
            printf("MPI_COMM_TYPE_HW_GUIDED: Trying invalid key - Passed\n");
        }
    }
    MPI_Info_free(&info);

    sync_hr();

    /*
     * Test MPI_COMM_TYPE_HW_GUIDED:
     *  - Test with "mpi_hw_resource_type" = "mpi_shared_memory"
     *  - Mix in some MPI_UNDEFINED values to make sure those are handled properly
     */
    expected_value = 3;
    if (expected_value > mcw_size) {
        expected_value = mcw_size;
    }
    MPI_Info_create(&info);
    MPI_Info_set(info, "mpi_hw_resource_type", "mpi_shared_memory");
    if (mcw_rank == 0 && verbose) {
        printf("MPI_COMM_TYPE_HW_GUIDED: Trying MPI Standard value %s with some MPI_UNDEFINED\n", "mpi_shared_memory");
    }
    if (mcw_rank < expected_value) {
        ret = MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_HW_GUIDED, 0, info, &comm);
    } else {
        ret = MPI_Comm_split_type(MPI_COMM_WORLD, MPI_UNDEFINED, 0, info, &comm);
    }
    if (ret != MPI_SUCCESS) {
        printf("MPI_COMM_TYPE_HW_GUIDED (%s) failed\n", split_topo[i]);
        errs++;
    } else if (comm != MPI_COMM_NULL) {
        MPI_Comm_rank(comm, &rank);
        MPI_Comm_size(comm, &size);
        value = 1;
        if (rank == 0 && verbose) {
            printf("MPI_COMM_TYPE_HW_GUIDED (%s): %d/%d Created shared subcommunicator of size %d\n",
                   "mpi_shared_memory", mcw_rank, mcw_size, size);
        }
        MPI_Comm_free(&comm);
    } else if (verbose) {
        value = 0;
        printf("MPI_COMM_TYPE_HW_GUIDED (%s): %d/%d Returned MPI_COMM_NULL\n",
               "mpi_shared_memory", mcw_rank, mcw_size);
    }
    MPI_Info_free(&info);

    if (mcw_rank == 0) {
        MPI_Reduce(MPI_IN_PLACE, &value, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        if (expected_value != value) {
            printf("MPI_COMM_TYPE_HW_GUIDED (%s): Failed: Verify expected %d == actual %d\n",
                   "mpi_shared_memory", expected_value, value);
        } else if (verbose) {
            printf("MPI_COMM_TYPE_HW_GUIDED (%s): Passed: Verify expected %d == actual %d\n",
                   "mpi_shared_memory", expected_value, value);
        }
    } else {
        MPI_Reduce(&value, NULL, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    sync_hr();

    /*
     * Test MPI_COMM_TYPE_HW_GUIDED:
     *  - info with correct key, but different values a different ranks, it must throw an error
     */
#if 0
    for (i = 0; NULL != split_topo[i]; i++) {
        ;
    }
    if (i < 2) {
        if (mcw_rank == 0 && verbose) {
            printf("MPI_COMM_TYPE_HW_GUIDED: Trying mismatched values -- SKIPPED not enough valid values (%d)\n", i);
        }
    } else {
        if (mcw_rank == 0 && verbose) {
            printf("MPI_COMM_TYPE_HW_GUIDED: Trying mismatched values\n");
        }
        MPI_Info_create(&info);
        if (mcw_rank == 0) {
            MPI_Info_set(info, "mpi_hw_resource_type", split_topo[0]);
        } else {
            MPI_Info_set(info, "mpi_hw_resource_type", split_topo[1]);
        }
        MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_HW_GUIDED, 0, info, &comm);
        if (comm != MPI_COMM_NULL) {
            printf("MPI_COMM_TYPE_HW_GUIDED with mismatched values key didn't return MPI_COMM_NULL\n");
            errs++;
            MPI_Comm_free(&comm);
        }
        MPI_Info_free(&info);
    }
#endif

    /* Test MPI_COMM_TYPE_HW_GUIDED:
     *  - info with semantically matching split_types and keys, it must throw an error
     */
#if 0
    if (mcw_rank == 0 && verbose) {
        printf("MPI_COMM_TYPE_HW_GUIDED: Trying mismatched keys but semantically matching\n");
    }

    if (mcw_rank == 0) {
        MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &comm);
    } else {
        MPI_Info_create(&info);
        MPI_Info_set(info, "mpi_hw_resource_type", "mpi_shared_memory");
        MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_HW_GUIDED, 0, info, &comm);
    }
    if (comm != MPI_COMM_NULL) {
        printf("MPI_COMM_TYPE_HW_GUIDED with mismatched values key didn't return MPI_COMM_NULL\n");
        errs++;
        MPI_Comm_free(&comm);
    }
    MPI_Info_free(&info);
#endif

    /* Test MPI_COMM_TYPE_HW_UNGUIDED:
     *  - TODO
     */
#if 0
    if (mcw_rank == 0 && verbose)
        printf("MPI_COMM_TYPE_HW_UNGUIDED: Trying basic\n");
    MPI_Info_create(&info);
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_HW_UNGUIDED, 0, info, &comm);
    if (comm != MPI_COMM_NULL) {
        int newsize;
        MPI_Comm_size(comm, &newsize);
        if (!(newsize < mcw_size)) {
            printf("MPI_COMM_TYPE_HW_UNGUIDED: Expected comm to be a proper sub communicator\n");
            errs++;
        }
        char resource_type[100] = "";
        int has_key = 0;
        MPI_Info_get(info, "mpi_hw_resource_type", 100, resource_type, &has_key);
        if (!has_key || strlen(resource_type) == 0) {
            printf("MPI_COMM_TYPE_HW_UNGUIDED: info for mpi_hw_resource_type not returned\n");
            errs++;
        }

        MPI_Comm_free(&comm);
    }
    else if (mcw_rank == 0 && verbose) {
        printf("MPI_COMM_TYPE_HW_UNGUIDED: Returned MPI_COMM_NULL\n");
    }
#endif

    /*
     * All done - figure out if we passed
     */
 done:
    MPI_Reduce(&errs, &tot_errs, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Finalize();

    return tot_errs;
}
