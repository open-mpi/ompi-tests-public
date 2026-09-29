#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void test_shared_memory(MPI_Win win, MPI_Comm comm, int *local_mem, int shared_size, const char *test_name) {
    int ret = MPI_SUCCESS;
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    // Configure the window to return errors instead of aborting
    MPI_Win_set_errhandler(win, MPI_ERRORS_RETURN);
    if (rank != 0) {
        do {
            // Query the shared memory of rank 0
            int *baseptr;
            MPI_Aint queried_size;
            int disp_unit;
            ret = MPI_Win_shared_query(win, 0, &queried_size, &disp_unit, &baseptr);
            if (ret != MPI_SUCCESS) {
                printf("%s: rank %d: MPI_Win_shared_query failed with error code %d\n", test_name, rank, ret);
                ret = MPI_ERR_OTHER;
                break;
            }

            if (baseptr == NULL) {
                printf("%s: rank %d: MPI_Win_shared_query returned NULL\n", test_name, rank);
                if (queried_size != 0) {
                    printf("%s: rank %d: Expected zero size but got %ld\n", test_name, rank, (long)queried_size);
                }
                ret = MPI_ERR_NO_MEM;
                break;
            }

            if (queried_size != shared_size) {
                printf("%s: rank %d: Queried size mismatch: expected %d, got %ld\n", test_name, rank, shared_size, (long)queried_size);
                ret = MPI_ERR_SIZE;
                break;
            }

            // Write data to the shared memory
            baseptr[rank] = 42;

        } while (false);
    }

    int *ret_vals = (int *)malloc(size * sizeof(int));
    if (ret_vals == NULL) {
        fprintf(stderr, "%s: rank %d: malloc for ret_vals failed\n", test_name, rank);
        MPI_Abort(comm, EXIT_FAILURE);
    }
    MPI_Gather(&ret, 1, MPI_INT, ret_vals, 1, MPI_INT, 0, comm);

    if (rank == 0) {
        int valid_procs = size - 1; // Exclude rank 0
        bool all_success = true;
        for (int i = 1; i < size; ++i) {
            if (ret_vals[i] != MPI_SUCCESS) {
                valid_procs--;
                continue; // Skip ranks that had errors
            }
            // Check if the data was written correctly
            if (local_mem[i] != 42) {
                all_success = false;
                printf("%s: rank 0: Data verification failed at %d!\n", test_name, i);
            }
        }
        if (all_success && valid_procs > 0) {
            printf("%s: rank 0: all data written correctly!\n", test_name);
        }
    }

    free(ret_vals);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int shared_size;
    int shared_rank;
    MPI_Comm comm_shared;
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, rank, MPI_INFO_NULL, &comm_shared);
    MPI_Comm_rank(comm_shared, &shared_rank);
    MPI_Comm_size(comm_shared, &shared_size);

    // Test with MPI_Win_allocate_shared
    MPI_Win win_shared;
    int *shared_mem_shared = NULL;
    int win_size = size*sizeof(int);
    MPI_Win_allocate_shared(shared_rank == 0 ? sizeof(int)*shared_size : 0, sizeof(int), MPI_INFO_NULL, comm_shared, &shared_mem_shared, &win_shared);
    test_shared_memory(win_shared, comm_shared, shared_mem_shared, sizeof(int)*shared_size, "MPI_Win_allocate_shared");
    MPI_Win_free(&win_shared);

    // Test with MPI_Win_allocate
    MPI_Win win_allocate;
    int *shared_mem_allocate = NULL;
    MPI_Win_allocate(rank == 0 ? win_size : 0, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &shared_mem_allocate, &win_allocate);
    test_shared_memory(win_allocate, MPI_COMM_WORLD, shared_mem_allocate, win_size, "MPI_Win_allocate");
    MPI_Win_free(&win_allocate);

    // Test with MPI_Win_create
    MPI_Win win_create;
    int* local_mem = (int*)malloc(win_size);
    if (local_mem == NULL) {
        fprintf(stderr, "MPI_Win_create: rank %d: malloc for local_mem failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    MPI_Win_create(rank == 0 ? local_mem : NULL, rank == 0 ? win_size : 0, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win_create);
    test_shared_memory(win_create, MPI_COMM_WORLD, local_mem, win_size, "MPI_Win_create");
    MPI_Win_free(&win_create);
    free(local_mem);

    MPI_Finalize();
    return 0;
}

