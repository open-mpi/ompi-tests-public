/*
 * Copyright (c) 2021-2022 IBM Corporation.  All rights reserved.
 *
 * $COPYRIGHT$
 */

/*
 * Default: Testing (TEST_PAYLOAD_SIZE / sizeof(type)) + 1
 *
 * Adjust TEST_PAYLOAD_SIZE from default value:
 *   -DTEST_PAYLOAD_SIZE=#
 * TEST_PAYLOAD_SIZE value is used as the numerator for each datatype.
 * The count used for that datatype is calculated as:
 *   (1 + (TEST_PAYLOAD_SIZE / sizeof(datatype)))
 * Using this variable one can test any code that guards against
 * a payload size that is too large.
 *
 * Adjust an individual size:
 *   -DV_SIZE_DOUBLE_COMPLEX=123
 *
 * Set the same count for all types:
 *   -DTEST_UNIFORM_COUNT=123
 */
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <complex.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#define PRIME_MODULUS 997

/*
 * Debugging messages
 * 0 = off
 * 1 = show displacements at root
 * 2 = show all checks
 */
int debug = 0;

/*
 * Valid after MPI_Init
 */
#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 255
#endif
int world_size = 0, world_rank = 0, local_size = 0;
char my_hostname[MPI_MAX_PROCESSOR_NAME];

/*
 * Limit how much total memory a collective can take on the system
 * across all processes.
 */
int max_sys_mem_gb = 0;

/*
 * Total memory on the system (for reference)
 */
int total_sys_mem_gb = 0;

/*
 * Tolerate this number of GB difference between systems
 */
int mem_diff_tolerance = 0;

/*
 * Percent of memory to allocate
 */
int mem_percent = 0;

/*
 * Allow the nonblocking tests to run
 */
bool allow_nonblocked = true;

/*
 * Algorithm expected inflation multiplier
 */
double alg_inflation = 1.0;

/*
 * 'v' collectives have two modes
 * Packed: contiguous packing of data
 * Skip  : a 'disp_stride' is created between every rank's contribution
 */
enum {
      MODE_PACKED = 1,
      MODE_SKIP   = 2
};

/*
 * Displacement between elements when not PACKED
 */
int disp_stride = 2;

/*
 * Define count paramters to use in the tests
 */
// Default: UINT_MAX 4294967295
//           INT_MAX 2147483647
#ifndef TEST_PAYLOAD_SIZE
#define TEST_PAYLOAD_SIZE UINT_MAX
#endif

#ifndef TEST_UNIFORM_COUNT
#ifndef V_SIZE_DOUBLE_COMPLEX
// double _Complex  = 16 bytes x  268435455.9375
#define V_SIZE_DOUBLE_COMPLEX    (int)(TEST_PAYLOAD_SIZE / sizeof(double _Complex))
#endif

#ifndef V_SIZE_DOUBLE
// double           =  8 bytes x  536870911.875
#define V_SIZE_DOUBLE            (int)(TEST_PAYLOAD_SIZE / sizeof(double))
#endif

#ifndef V_SIZE_FLOAT_COMPLEX
// float _Complex   =  8 bytes x  536870911.875
#define V_SIZE_FLOAT_COMPLEX     (int)(TEST_PAYLOAD_SIZE / sizeof(float _Complex))
#endif

#ifndef V_SIZE_FLOAT
// float            =  4 bytes x 1073741823.75
#define V_SIZE_FLOAT             (int)(TEST_PAYLOAD_SIZE / sizeof(float))
#endif

#ifndef V_SIZE_INT
// int              =  4 bytes x 1073741823.75
#define V_SIZE_INT               (int)(TEST_PAYLOAD_SIZE / sizeof(int))
#endif

#else
#define V_SIZE_DOUBLE_COMPLEX    TEST_UNIFORM_COUNT
#define V_SIZE_DOUBLE            TEST_UNIFORM_COUNT
#define V_SIZE_FLOAT_COMPLEX     TEST_UNIFORM_COUNT
#define V_SIZE_FLOAT             TEST_UNIFORM_COUNT
#define V_SIZE_INT               TEST_UNIFORM_COUNT
#define V_SIZE_CHAR              TEST_UNIFORM_COUNT
#endif

/*
 * Wrapper around 'malloc' that errors out if we cannot allocate the buffer.
 *
 * @param sz size of the buffer
 * @return pointer to the memory. Does not return on error.
 */
static inline void * safe_malloc(size_t sz)
{
    void * ptr = NULL;
    ptr = malloc(sz);
    if( NULL == ptr ) {
        fprintf(stderr, "Rank %d on %s) Error: Failed to malloc(%zu)\n", world_rank, my_hostname, sz);
#ifdef MPI_VERSION
        MPI_Abort(MPI_COMM_WORLD, 3);
#else
        exit(ENOMEM);
#endif
    }
    return ptr;
}

/*
 * Convert a value in whole bytes to the abbreviated form
 *
 * @param value size in whole bytes
 * @return static string representation of the whole bytes in the nearest abbreviated form
 */
static inline const char * human_bytes(size_t value)
{
    static char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
    static int s_len = 5;
    static char h_out[30];
    int s_idx = 0;
    double d_value = value;

    if( value > 1024 ) {
        for( s_idx = 0; s_idx < s_len && d_value > 1024; ++s_idx ) {
            d_value = d_value / 1024.0;
        }
    }

    snprintf(h_out, 30, "%2.1f %s", d_value, suffix[s_idx]);
    return h_out;
}

/*
 * Determine amount of memory to use, in GBytes as a percentage of total physical memory
 *
 * @return Amount of memory to use (in GB)
 */
static int get_max_memory(void) {
    char *mem_percent_str;
    char *endp;
    FILE *meminfo_file;
    char *proc_data;
    char *token;
    size_t bufsize;
    int mem_to_use;
    int rc;

    mem_percent_str = getenv("BIGCOUNT_MEMORY_PERCENT");
    if (NULL == mem_percent_str) {
        mem_percent_str = "80";
    }

    mem_percent = strtol(mem_percent_str, &endp, 10);
    if ('\0' != *endp) {
        fprintf(stderr, "BIGCOUNT_MEMORY_PERCENT is not numeric\n");
        exit(1);
    }

    meminfo_file = fopen("/proc/meminfo", "r");
    if (NULL == meminfo_file) {
        fprintf(stderr, "Unable to open /proc/meminfo file: %s\n", strerror(errno));
        exit(1);
    }

    bufsize = 0;
    proc_data = NULL;
    mem_to_use = 0;
    rc = getline(&proc_data, &bufsize, meminfo_file);
    while (rc > 0) {
        token = strtok(proc_data, " ");
        if (NULL != token) {
            if (!strcmp(token, "MemTotal:")) {
                token = strtok(NULL, " ");
                total_sys_mem_gb = strtol(token, NULL, 10);
                total_sys_mem_gb = (int)(total_sys_mem_gb / 1048576.0);
                /* /proc/meminfo specifies memory in KBytes, convert to GBytes */
                mem_to_use = (int)(total_sys_mem_gb * (mem_percent / 100.0));
                break;
             }
        }
        rc = getline(&proc_data, &bufsize, meminfo_file);
    }

    if (0 == mem_to_use) {
        fprintf(stderr, "Unable to determine memory to use\n");
        exit(1);
    }

    free(proc_data);
    fclose(meminfo_file);
    return mem_to_use;
}

/*
 * Display a diagnostic table
 */
static inline void display_diagnostics(void) {
    printf("----------------------:-----------------------------------------\n");
    printf("Total Memory Avail.   : %4d GB\n", total_sys_mem_gb);
    printf("Percent memory to use : %4d %%\n", mem_percent);
    printf("Tolerate diff.        : %4d GB\n", mem_diff_tolerance);
    printf("Max memory to use     : %4d GB\n", max_sys_mem_gb);
    printf("----------------------:-----------------------------------------\n");
    printf("INT_MAX               : %20zu\n", (size_t)INT_MAX);
    printf("UINT_MAX              : %20zu\n", (size_t)UINT_MAX);
    printf("SIZE_MAX              : %20zu\n", (size_t)SIZE_MAX);
    printf("----------------------:-----------------------------------------\n");
    printf("                      : Count x Datatype size      = Total Bytes\n");
#ifndef TEST_UNIFORM_COUNT
    printf("TEST_PAYLOAD_SIZE     : %20zu       = %10s\n", (size_t)TEST_PAYLOAD_SIZE, human_bytes((size_t)TEST_PAYLOAD_SIZE));
#else
    printf("TEST_UNIFORM_COUNT    : %20zu\n", (size_t)TEST_UNIFORM_COUNT);
#endif
    printf("V_SIZE_DOUBLE_COMPLEX : %20zu x %3zu = %10s\n", (size_t)V_SIZE_DOUBLE_COMPLEX, sizeof(double _Complex), human_bytes(V_SIZE_DOUBLE_COMPLEX * sizeof(double _Complex)));
    printf("V_SIZE_DOUBLE         : %20zu x %3zu = %10s\n", (size_t)V_SIZE_DOUBLE, sizeof(double), human_bytes(V_SIZE_DOUBLE * sizeof(double)));
    printf("V_SIZE_FLOAT_COMPLEX  : %20zu x %3zu = %10s\n", (size_t)V_SIZE_FLOAT_COMPLEX, sizeof(float _Complex), human_bytes(V_SIZE_FLOAT_COMPLEX * sizeof(float _Complex)));
    printf("V_SIZE_FLOAT          : %20zu x %3zu = %10s\n", (size_t)V_SIZE_FLOAT, sizeof(float), human_bytes(V_SIZE_FLOAT * sizeof(float)));
    printf("V_SIZE_INT            : %20zu x %3zu = %10s\n", (size_t)V_SIZE_INT, sizeof(int), human_bytes(V_SIZE_INT * sizeof(int)));
    printf("----------------------:-----------------------------------------\n");
}

/*
 * Initialize the unit testing environment
 * Note: Must be called after MPI_Init()
 *
 * @param argc Argument count
 * @param argv Array of string arguments
 * @return 0 on success
 */
int init_environment(int argc, char** argv) {
    max_sys_mem_gb = get_max_memory();

#ifdef MPI_VERSION
    int i;
    int *per_local_sizes = NULL;
    int *local_max_mem = NULL;
    char *mem_diff_tolerance_str = NULL;
    char *env_str = NULL;
    int min_mem = 0;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Get_processor_name(my_hostname, &i);

    if( NULL != getenv("OMPI_COMM_WORLD_LOCAL_SIZE") ) {
        local_size = (int)strtol(getenv("OMPI_COMM_WORLD_LOCAL_SIZE"), NULL, 10);
    } else {
        local_size = world_size;
    }

    if( NULL != (env_str = getenv("BIGCOUNT_ENABLE_NONBLOCKING")) ) {
        if( 'y' == env_str[0] || 'Y' == env_str[0] || '1' == env_str[0] ) {
            allow_nonblocked = true;
        } else {
            allow_nonblocked = false;
        }
    }

    if( NULL != (env_str = getenv("BIGCOUNT_ALG_INFLATION")) ) {
        alg_inflation = strtod(env_str, NULL);
    }

    // Make sure that the local size is uniform
    if( 0 == world_rank ) {
        per_local_sizes = (int*)safe_malloc(sizeof(int) * world_size);
    }

    MPI_Gather(&local_size, 1, MPI_INT, per_local_sizes, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if( 0 == world_rank ) {
        for(i = 0; i < world_size; ++i) {
            if( local_size != per_local_sizes[i] ) {
                printf("Error: Non-uniform local size at peer %d : actual %d vs expected %d\n",
                       i, per_local_sizes[i], local_size);
                assert(local_size == per_local_sizes[i]);
            }
        }
        free(per_local_sizes);
    }
    // Make sure max memory usage is the same for all tasks
    if( 0 == world_rank ) {
        local_max_mem = (int*)safe_malloc(sizeof(int) * world_size);
    }

    mem_diff_tolerance_str = getenv("BIGCOUNT_MEMORY_DIFF");
    if (NULL != mem_diff_tolerance_str) {
        mem_diff_tolerance = strtol(mem_diff_tolerance_str, NULL, 10);
    }

    MPI_Gather(&max_sys_mem_gb, 1, MPI_INT, local_max_mem, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if( 0 == world_rank ) {
        min_mem = max_sys_mem_gb;
        for(i = 0; i < world_size; ++i) {
            if( max_sys_mem_gb != local_max_mem[i] ) {
                if( (max_sys_mem_gb < local_max_mem[i] && max_sys_mem_gb + mem_diff_tolerance < local_max_mem[i]) ||
                    (max_sys_mem_gb > local_max_mem[i] && max_sys_mem_gb > mem_diff_tolerance + local_max_mem[i]) ) {
                    printf("Error: Non-uniform max memory usage at peer %d : actual %d vs expected %d (+/- %d)\n",
                           i, local_max_mem[i], max_sys_mem_gb, mem_diff_tolerance);
                    assert(max_sys_mem_gb == local_max_mem[i]);
                }
                if( min_mem > local_max_mem[i] ) {
                    min_mem = local_max_mem[i];
                }
            }
        }
        free(local_max_mem);
        if( min_mem != max_sys_mem_gb ) {
            printf("Warning: Detected difference between local and remote available memory. Adjusting to: %d GB\n",
                   min_mem);
            max_sys_mem_gb = min_mem;
        }
    }

    // Agree on the max memory usage value to use across all processes
    MPI_Bcast(&max_sys_mem_gb, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
#else
    world_size = world_rank = local_size = -1;
    snprintf(my_hostname, MPI_MAX_PROCESSOR_NAME, "localhost");
#endif

    if( 0 == world_rank || -1 == world_rank ) {
        display_diagnostics();
    }

    return 0;
}

/*
 * Calculate the uniform count for this collective given the datatype size,
 * number of processes (local and global), expected inflation in memory during
 * the collective, and the amount of memory we are limiting this test to consuming
 * on the system.
 *
 * @param datateye_size size of the datatype
 * @param proposed_count the count that the caller wishes to use
 * @param mult_root memory multiplier at root (useful in gather-like operations where the root gathers N times the count)
 * @param mult_peer memory multiplier at non-roots (useful in allgather-like operations where the buffer is N times count)
 * @return proposed count to use in the collective
 */
size_t calc_uniform_count(size_t datatype_size, size_t proposed_count,
                          size_t mult_root, size_t mult_peer)
{
    size_t orig_proposed_count = proposed_count;
    size_t orig_mult_root = mult_root;
    size_t orig_mult_peer = mult_peer;
    size_t payload_size_root;
    size_t payload_size_peer;
    size_t payload_size_all;
    double perc = 1.0;
    char *cpy_root = NULL, *cpy_peer = NULL;

    mult_root = (size_t)(mult_root * alg_inflation);
    mult_peer = (size_t)(mult_peer * alg_inflation);

    payload_size_root = datatype_size * proposed_count * mult_root;
    payload_size_peer = datatype_size * proposed_count * mult_peer;
    payload_size_all  = payload_size_root + (payload_size_peer * (local_size-1));

    while( (payload_size_all / ((size_t)1024 * 1024 * 1024)) > max_sys_mem_gb ) {
        if( 2 == debug && 0 == world_rank ) {
            fprintf(stderr, "----DEBUG---- Adjusting count. Try count of %10zu (payload_size %4zu GB) to fit in %4d GB limit (perc %6.2f)\n",
                   proposed_count, payload_size_all/((size_t)1024 * 1024 * 1024), max_sys_mem_gb, perc);
        }

        perc -= 0.05;
        // It is possible that we are working with extremely limited memory
        // so the percentage dropped below 0. In this case just make the
        // percentage equal to the max_sys_mem_gb.
        if( perc <= 0.0 ) {
            proposed_count = (max_sys_mem_gb * (size_t)1024 * 1024 * 1024) / (datatype_size * mult_root + datatype_size * mult_peer * (local_size-1));
            perc = proposed_count / (double)orig_proposed_count;
            if( 2 == debug && 0 == world_rank ) {
                fprintf(stderr, "----DEBUG---- Adjusting count. Try count of %10zu (from %10zu) to fit in %4d GB limit (perc %6.9f) -- FINAL\n",
                       proposed_count, orig_proposed_count, max_sys_mem_gb, perc);
            }
        } else {
            proposed_count = orig_proposed_count * perc;
        }
        assert(perc > 0.0);

        payload_size_root = datatype_size * proposed_count * mult_root;
        payload_size_peer = datatype_size * proposed_count * mult_peer;
        payload_size_all  = payload_size_root + (payload_size_peer * (local_size-1));
    }

    if(proposed_count != orig_proposed_count ) {
        if( 0 == world_rank ) {

            printf("--------------------- Adjust count to fit in memory: %10zu x %5.1f%% = %10zu\n",
                   orig_proposed_count,
                   (proposed_count / (double)orig_proposed_count)*100,
                   proposed_count);

            cpy_root = strdup(human_bytes(payload_size_root));
            printf("Root  : payload %14zu %8s = %3zu dt x %10zu count x %3zu peers x %5.1f inflation\n",
                   payload_size_root, cpy_root,
                   datatype_size, proposed_count, orig_mult_root, alg_inflation);

            cpy_peer = strdup(human_bytes(payload_size_peer));
            printf("Peer  : payload %14zu %8s = %3zu dt x %10zu count x %3zu peers x %5.1f inflation\n",
                   payload_size_peer, cpy_peer,
                   datatype_size, proposed_count, orig_mult_peer, alg_inflation);

            printf("Total : payload %14zu %8s = %8s root + %8s x %3d local peers\n",
                   payload_size_all, human_bytes(payload_size_all),
                   cpy_root, cpy_peer, local_size-1);

            free(cpy_root);
            free(cpy_peer);
        }
    }

    return proposed_count;
}
