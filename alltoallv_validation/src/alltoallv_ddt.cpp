/*
 * Copyright (c) 2024      Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Additional copyrights may follow
 *
 */


#include <iostream>
#include <chrono>
#include <random>

#include <mpi.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

extern "C" void printMapDatatype(MPI_Datatype datatype);


#define NUM_LEVEL1_TESTS 9
#define NUM_LEVEL2_TESTS 7
#define NUM_LEVEL3_TESTS 6


#define ERROR_CHECK( err, errlab ) if(err) { printf("ERROR: An error (%d) in an MPI call was detected at %s:%d!\n", err, __FILE__, __LINE__); goto errlab; }
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

int execute_test(struct run_config *run);

#define VERBOSE_LEVEL_QUIET (user.verbose == 0)
#define VERBOSE_LEVEL_DEFAULT (user.verbose >= 1)
#define VERBOSE_LEVEL_LOUD (user.verbose >= 2)
#define VERBOSE_LEVEL_VERY_LOUD (user.verbose >= 3)

struct user_config
{
    int seed = 0;
    int item_count = 10;
    int iters = 2;
    /* probability that rank r will send item k to rank q: */
    double prob_item = 0.50;
    /* probability that rank r will send anything to rank q: */
    double prob_rank = 0.85;
    /* probability that rank r will send or receive anything at all: */
    double prob_world = 0.9;

    /* verbose: 0 is very quiet.  1 is default. 2 is loud 3 is very loud. */
    int verbose = 0;
    int only_high = 0;
    int only_low = 0;
};

static struct user_config user;

size_t tot_bytes_sent = 0;
size_t tot_bytes_recv = 0;
size_t tot_tests_exec = 0;

void *my_malloc(size_t size) {
    if (size==0) size++;
    return malloc(size);
}
#define malloc my_malloc

void dump_user_config(struct user_config *conf) {
    std::cout << "seed: " << conf->seed << "\n";
    std::cout << "items-count: " << conf->item_count << "\n";
    std::cout << "prob-item: " << conf->prob_item << "\n";
    std::cout << "prob-rank: " << conf->prob_rank << "\n";
    std::cout << "prob-world: " << conf->prob_world << "\n";
    std::cout << "verbose: " << conf->verbose << "\n";
}

void dump_type_info(MPI_Datatype dtype, const char *label) {
    MPI_Aint lb, extent, true_lb, true_extent;
    int size;

    MPI_Type_get_extent(dtype, &lb, &extent);
    MPI_Type_get_true_extent(dtype, &true_lb, &true_extent);
    MPI_Type_size(dtype, &size);
    printf("%s: Extent %ld (true: %ld). LB %ld (true: %ld).  Size %d\n",label,
        extent, true_extent, lb, true_lb, size);
    printMapDatatype(dtype);
}

struct run_config
{
    struct user_config *user;

    uint8_t *send_mat;
    uint8_t *recv_mat;

    int *sendcounts;
    int *recvcounts;
    int *sdispls;
    int *rdispls;
    int *remote_sdispls;
    size_t sum_send_count;
    size_t sum_recv_count;

    MPI_Datatype sdtype;
    MPI_Datatype rdtype;
    int sdcount_mult;
    int rdcount_mult;
};

void print_help()
{
    printf("Test alltoallv using various ddt's and validate results.\n");
    printf("This test uses pseudo-random sequences from C++'s mt19937 generator.\n");
    printf("The test (but not necessarily the implementation) is deterministic\n");
    printf("when the options and number of ranks remain the same.\n");
    printf("Options:\n");
    printf("\t [-s|--seed <seed>]           Change the seed to shuffle which datapoints are exchanged\n");
    printf("\t [-c|--item-count <citems>]   Each rank will create <citems> to consider for exchange (default=10).\n");
    printf("\t [-i|--prob-item <prob>]      Probability that rank r will send item k to rank q. (0.50)\n");
    printf("\t [-r|--prob-rank <prob>]      Probability that rank r will send anything to rank q. (0.90)\n");
    printf("\t [-w|--prob-world <prob>]     Probability that rank r will do anything at all. (0.95)\n");
    printf("\t [-t|--iters <iters>]         The number of iterations to test each dtype.\n");
    printf("\t [-o|--only <high,low>]       Only execute a specific test signified by the pair high,low.\n");
    printf("\t                              low=0 means run all tests in that high level\n");
    printf("\t [-v|--verbose=level ]        Set verbosity during execution (0=quiet (default). 1,2,3: loud).\n");
    printf("\t [-h|--help]                  Print this help and exit.\n");
    printf("\t [-z|--verbose-rank]          Only the provided rank will print.  Default=0.  ALL = -1.\n");

    printf("\n");
}

int level1_types( int jtest, MPI_Datatype *dtype) {
    switch (jtest) {
        case 0:  *dtype = MPI_CHAR;        break;
        case 1:  *dtype = MPI_REAL;        break;
        case 2:  *dtype = MPI_INT;         break;
        case 3:  *dtype = MPI_INT8_T;      break;
        case 4:  *dtype = MPI_INT16_T;     break;
        case 5:  *dtype = MPI_INT32_T;     break;
        case 6:  *dtype = MPI_INT64_T;     break;
        case 7:  *dtype = MPI_REAL4;       break;
        case 8:  *dtype = MPI_REAL8;       break;
        case NUM_LEVEL1_TESTS:
        default:
            dtype = NULL;
            return 1;
    }
    return 0;
}


bool is_predefined_type(MPI_Datatype dtype) {
    MPI_Datatype dt_test;
    for (int j=0; j<NUM_LEVEL1_TESTS; j++) {
        if (level1_types(j, &dt_test))
            return false;
        if (dtype == dt_test)
            return true;
    }
    return false;
}

int level2_types( int jtest, int length, MPI_Datatype basetype, MPI_Datatype *sdtype, int *sdcount) {
    int nblocks, per_block;
    if (length % 12 == 0) {
        per_block = length/12;
        nblocks = 12;
    } else if (length % 5 == 0) {
        nblocks = length/5;
        per_block = 5;
    } else if (length % 3 == 0) {
        nblocks = length/3;
        per_block = 3;
    } else if (length % 2 == 0) {
        nblocks = length/2;
        per_block = 2;
    } else {
        nblocks = length;
        per_block = 1;
    }
    int err;
    switch (jtest) {
        case 0:
            *sdtype = basetype;
            *sdcount = length;
            break;
        case 1:
            /* simple contiguous */
            err = MPI_Type_contiguous(length, basetype, sdtype);
            ERROR_CHECK(err, on_error);
            *sdcount = 1;
            break;
        case 2:
            /* equivalent to contiguous */
            err = MPI_Type_vector(nblocks, per_block, per_block, basetype, sdtype);
            ERROR_CHECK(err, on_error);
            *sdcount = 1;
            break;
        case 3:
            /* blocks with 1 empty space between them */
            err = MPI_Type_vector(nblocks, per_block, per_block+1, basetype, sdtype);
            ERROR_CHECK(err, on_error);
            *sdcount = 1;
            break;
        case 4:
            /* blocks with exactly half the space filled */
            err = MPI_Type_vector(nblocks, per_block, per_block*2, basetype, sdtype);
            ERROR_CHECK(err, on_error);
            *sdcount = 1;
            break;
        case 5:
            /* a contiguous block going backwards */
            err = MPI_Type_vector(nblocks, per_block, -per_block, basetype, sdtype);
            ERROR_CHECK(err, on_error);
            *sdcount = 1;
            break;
        case 6:
            /* a block going backwards with some extra empty space between them */
            err = MPI_Type_vector(nblocks, per_block, -per_block-1, basetype, sdtype);
            ERROR_CHECK(err, on_error);
            *sdcount = 1;
            break;
        case NUM_LEVEL2_TESTS:
        default:
            return -1;
    }
    if (VERBOSE_LEVEL_VERY_LOUD) {
        MPI_Aint lb, extent, true_lb, true_extent;
        MPI_Type_get_extent(*sdtype, &lb, &extent);
        MPI_Type_get_true_extent(*sdtype, &true_lb, &true_extent);
        printf("Created span from %ld:%ld.  Data from %ld:%ld\n",
                lb, lb+extent, true_lb, true_lb+true_extent);
    }

    return 0;
    on_error:
    return 1;
}

int level3_types( int jtest, MPI_Datatype basetypeA, MPI_Datatype basetypeB, MPI_Datatype *sdtype) {

    MPI_Aint lbA, lbB, extentA, extentB;
    MPI_Aint true_lbA, true_lbB, true_extentA, true_extentB;
    int err, blocklens[2];
    MPI_Aint displs[2];
    MPI_Datatype dtypes[2];

    dtypes[0] = basetypeA;
    dtypes[1] = basetypeB;
    blocklens[0] = 1;
    blocklens[1] = 1;

    err = MPI_Type_get_extent(basetypeA, &lbA, &extentA);
    err = MPI_Type_get_true_extent(basetypeA, &true_lbA, &true_extentA);

    err = MPI_Type_get_extent(basetypeB, &lbB, &extentB);
    err = MPI_Type_get_true_extent(basetypeB, &true_lbB, &true_extentB);
    ERROR_CHECK(err, on_error);

    switch (jtest) {
        case 0:
            // A first (at 0), then B, no space.
            displs[0] = -true_lbA;
            displs[1] = -true_lbB + true_extentA;
            break;
        case 1:
            // B first, then A, no space.
            displs[0] = -lbA + true_extentB;
            displs[1] = -true_lbB;
            break;
        case 2:
            // A first, then B, ref to A's LB.
            displs[0] = 0;
            displs[1] = +lbA -lbB + extentA;
            break;
        case 3:
            // B first, then A, ref to B's LB.
            displs[0] = +lbA -lbB + extentB;
            displs[1] = 0;
            break;
        case 4:
            // A first, then B, Starting at -11 and with extra space.
            displs[0] = -11-lbA;
            displs[1] =   0 -true_lbB + extentA;
            break;
        case 5:
            // Same as 0, but we'll resize it below.
            displs[0] = -true_lbA;
            displs[1] = -true_lbB + true_extentA;
            break;
        case NUM_LEVEL3_TESTS:
        default:
            return -1;
    }

    err = MPI_Type_create_struct(2, blocklens, displs, dtypes, sdtype);
    ERROR_CHECK(err, on_error);

    if (jtest == 5) {
        MPI_Datatype old = *sdtype;
        MPI_Aint lb, extent;
        MPI_Type_get_extent(old, &lb, &extent);
        MPI_Type_create_resized(old, -13+lb, extent+13, sdtype);
    }
    if (VERBOSE_LEVEL_VERY_LOUD) {
        MPI_Type_get_extent(*sdtype, &lbA, &extentA);
        MPI_Type_get_true_extent(*sdtype, &true_lbA, &true_extentA);
        printf("Created A-B span from %ld:%ld.  Data from %ld:%ld\n",
                lbA, lbA+extentA, true_lbA, true_lbA+true_extentA);
    }

    return err;

    on_error:
    return 1;
}

int top_level_exhaustive(struct run_config *run) {
    int rank;
    int err;
    int low_counter;

    run->sdcount_mult = 1;
    run->rdcount_mult = 1;
    err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    ERROR_CHECK(err, on_error);

    low_counter = 0;
    /*
    Level1:

    These test various basic types on their own.  We cannot send one type and
    receive another, so this is a single loop
    */
    for (int jd=0; jd<NUM_LEVEL1_TESTS; jd++) {
        low_counter++;
        if (run->user->only_high && run->user->only_high != 1) break;
        if (run->user->only_low && run->user->only_low != low_counter) continue;
        if (VERBOSE_LEVEL_DEFAULT) printf("--- Starting test 1,%d\n",low_counter);

        err = level1_types(jd, &run->sdtype);
        ERROR_CHECK(err, on_error);
        err = level1_types(jd, &run->rdtype);
        ERROR_CHECK(err, on_error);
        err = execute_test(run);
        ERROR_CHECK(err, on_error);


        MPI_Barrier(MPI_COMM_WORLD);
    }

    run->sdcount_mult = 1;
    run->rdcount_mult = 1;
    low_counter = 0;
    /*
    Level2:

    These test multiples of a single type in various forms including
    non-contiguous.  As long as the base type is the same (and we just re-use
    MPI_INT) then these types are compatible with each other, so the double loop
    makes sure they can all inter-operate.
    */
    for (int js=0; js<NUM_LEVEL2_TESTS; js++) {
        if (run->user->only_high && run->user->only_high != 2) break;
    for (int jr=0; jr<NUM_LEVEL2_TESTS; jr++) {
        low_counter++;
        if (run->user->only_low && run->user->only_low != low_counter) continue;
        if (VERBOSE_LEVEL_DEFAULT) printf("--- Starting test 2,%d.  Crossing %d x %d\n",low_counter, js, jr);

        err = level2_types( js, 12, MPI_INT, &run->sdtype, &run->sdcount_mult);
        err |= level2_types( jr, 12, MPI_INT, &run->rdtype, &run->rdcount_mult);
        ERROR_CHECK(err, on_error);

        err = execute_test(run);
        ERROR_CHECK(err, on_error);
        if (!is_predefined_type(run->sdtype)) {
            MPI_Type_free( &run->sdtype );
        }
        if (!is_predefined_type(run->rdtype)) {
            MPI_Type_free( &run->rdtype );
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    }

    run->rdcount_mult = 1;
    run->sdcount_mult = 1;
    low_counter = 0;
    for (int js=0; js<NUM_LEVEL3_TESTS; js++) {
    for (int jr=0; jr<NUM_LEVEL3_TESTS; jr++) {
        low_counter++;
        if (run->user->only_high && run->user->only_high != 3) break;
        if (run->user->only_low && run->user->only_low != low_counter) continue;
        if (VERBOSE_LEVEL_DEFAULT) printf("--- Starting test 3,%d.  Crossing %d x %d\n",low_counter, js, jr);
        err = level3_types( js, MPI_INT, MPI_CHAR, &run->sdtype);
        err |= level3_types( jr, MPI_INT, MPI_CHAR, &run->rdtype);
        ERROR_CHECK(err, on_error);
        err = execute_test(run);
        ERROR_CHECK(err, on_error);
        if (!is_predefined_type(run->sdtype)) {
            err = MPI_Type_free( &run->sdtype );
            ERROR_CHECK(err, on_error);
        }
        if (!is_predefined_type(run->rdtype)) {
            err = MPI_Type_free( &run->rdtype );
            ERROR_CHECK(err, on_error);
        }
        err = MPI_Barrier(MPI_COMM_WORLD);
        ERROR_CHECK(err, on_error);
    }
    }

    low_counter = 0;
    do {
        low_counter++;
        if (run->user->only_high && run->user->only_high != 4) break;
        if (run->user->only_low && run->user->only_low != low_counter) continue;
        if (VERBOSE_LEVEL_DEFAULT) printf("--- Starting test 4,%d\n",low_counter);
        if (low_counter == 1) {
            int blk_lens[2];
            MPI_Aint blk_displ[2];
            MPI_Datatype blk_types[2];
            MPI_Datatype dtype;
            blk_lens[0] = 1;
            blk_lens[1] = 1;
            blk_displ[0] = -4;
            blk_displ[1] = 4;
            blk_types[0] = MPI_CHAR;
            blk_types[1] = MPI_CHAR;

            err = MPI_Type_create_struct(2, blk_lens, blk_displ, blk_types, &dtype);
            ERROR_CHECK(err, on_error);
            err = MPI_Type_commit(&dtype);
            ERROR_CHECK(err, on_error);
            if (rank != 0) {
                run->rdcount_mult = 1;
                run->rdtype = dtype;
                run->sdcount_mult = 1;
                run->sdtype = dtype;
            } else {
                run->rdcount_mult = 2;
                run->rdtype = MPI_CHAR;
                run->sdcount_mult = 2;
                run->sdtype = MPI_CHAR;
            }

            err = execute_test(run);
            ERROR_CHECK(err, on_error);


        } else if (low_counter == 2) {

            MPI_Datatype send_pile[4];
            MPI_Datatype recv_pile[9];
            int ignored;

            /**
             * In case you have to debug this, it should look something like this:
             * Sender
             *  Vector 48 long of: ((real,char),(char,int64))
             *
             * Reciever
             * (
             *      (
             *          Vector 36 long of: ((real,char),(char,int64))
             *          (real,char),(char,int64))
             *      ),
             *      (
             *          Vector 10 long of: ((real,char),(char,int64)),
             *          (real,char),(char,int64))
             *      )
             * )
             * And of course, the layouts of each of those things is all messy.
             */

            // level3_types( 5, MPI_INT, MPI_CHAR, &send_pile[0]);
            // level3_types( 6, MPI_INT, MPI_CHAR, &recv_pile[0]);

            level3_types( 4, MPI_CHAR, MPI_CHAR, &send_pile[0]);
            level3_types( 3, MPI_CHAR, MPI_CHAR, &recv_pile[0]);

            level3_types( 2, MPI_CHAR, MPI_CHAR, &send_pile[1]);
            level3_types( 1, MPI_CHAR, MPI_CHAR, &recv_pile[1]);

            level3_types( 2, send_pile[0], send_pile[1], &send_pile[2]);
            level3_types( 2, recv_pile[0], recv_pile[1], &recv_pile[2]);

            /* create our vector: note that level1 tests other than 0 don't use the mult so we ignore it. */
            level2_types( 5, 48, send_pile[2], &send_pile[3], &ignored);

            // /* create two vectors, totaling 46, then two extra items to add up to 48.*/
            level2_types( 3, 36, recv_pile[2], &recv_pile[3], &ignored);
            level2_types( 1, 10, recv_pile[2], &recv_pile[4], &ignored);
            level3_types( 5, recv_pile[3], recv_pile[2], &recv_pile[6]);
            level3_types( 5, recv_pile[4], recv_pile[2], &recv_pile[7]);
            level3_types( 5, recv_pile[6], recv_pile[7], &recv_pile[8]);

            run->rdcount_mult = 1;
            run->sdcount_mult = 1;
            run->sdtype = send_pile[3];
            // run->rdtype = recv_pile[8];
            run->rdtype = send_pile[3];
            // run->sdtype = send_pile[2];
            // run->rdtype = recv_pile[2];
            MPI_Type_commit(&run->sdtype);
            MPI_Type_commit(&run->rdtype);

            err = execute_test(run);
            ERROR_CHECK(err, on_error);
        }

    } while (low_counter+1 <= 2);

    return 0;
    on_error:
    return -1;
}


/* simple pattern so we can check it later.  We only care about byte position.
 Note: reserve 0 and 1, so we can memset those as "holes" in the send and recv
 message buffers respectively. */
void fill_pattern_buf(uint8_t *buf, size_t nbytes, int rank, int iter) {
    uint8_t last_val = iter;
    for (int jbyte = 0; jbyte<nbytes; jbyte++) {
        buf[jbyte] = last_val < 2 ? 255 : last_val;
        last_val += (rank+1);
    }
}
/* Must match with the above fill_pattern_buf, requires the sdispls used by the
   remote to send to us so we know where to pick up the remote's pattern
   generation. */
int check_pattern_buf( uint8_t *buf, int type_size, int comm_size, int* rcounts, int *remote_sdispls, int jiter) {
    int rank;
    int rc;
    rc = 0;
    uint8_t *expected_buf;
    int total_size=0;
    for (int jrank=0; jrank < comm_size; jrank++) {
        total_size += type_size * rcounts[jrank];
    }
    expected_buf = (uint8_t*)malloc(total_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int jbyte_buf = 0;
    for (int jrank=0; jrank < comm_size; jrank++) {
        size_t bytes_remaining = type_size * rcounts[jrank];
        size_t byte_offset_at_sender = type_size * remote_sdispls[jrank];
        uint8_t last_val = jiter + (jrank+1) * byte_offset_at_sender;
        /* uncomment in case of error: */
        // printf("Rank %d checking data from Rank %d: %ld bytes (%d sz * %d counts).  Remote started at byte %ld\n", rank, jrank, bytes_remaining, type_size, rcounts[jrank], byte_offset_at_sender);
        while (bytes_remaining && jbyte_buf < total_size) {
            uint8_t compare_val = last_val < 2 ? 255 : last_val;
            expected_buf[jbyte_buf] = compare_val;
            if (compare_val != buf[jbyte_buf]) {
                int rank;
                MPI_Comm_rank(MPI_COMM_WORLD, &rank);
                // printf("ERROR: Rank %d: byte[%d] != %d.  Found %d\n", rank, jbyte_buf, compare_val, buf[jbyte_buf]);
                rc = 1;
            }
            jbyte_buf++;
            bytes_remaining--;
            last_val += (jrank+1);
        }
    }
    if ( rc ) {
        char line_buf[1024];
        char *line0;
        int last_ok;
        int this_ok;
        last_ok = -1;
        this_ok = 1;
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        printf("Rank %d failed to validate data!\n",rank);
        if (VERBOSE_LEVEL_LOUD) {
            const char *ok_str = "-- VALID";
            line0 = line_buf;
            for (int jbyte=0; jbyte<total_size; jbyte++) {
                if (jbyte % 10 == 0) {
                    if (jbyte && last_ok != this_ok) {
                        printf("%s%s\n",line_buf, ok_str);
                    }
                    line0 = line_buf;
                    last_ok = this_ok;
                    this_ok = 1;
                    line0 += sprintf(line0,"%04d: ",jbyte);
                    ok_str = "-- VALID";
                }
                if (buf[jbyte] != expected_buf[jbyte]) {
                    ok_str = "-- CORRUPT";
                    this_ok = 0;
                }
                line0 += sprintf(line0,"%3d-%-3d ",buf[jbyte], expected_buf[jbyte]);
            }
        }
    }
    free(expected_buf);
    return rc;
}

void set_guard_bytes(uint8_t* guards[4], int guard_length, int guard_val) {
    for (int j=0; j<4; j++) {
        for (int k=0; k<guard_length; k++) {
            guards[j][k] = guard_val;
        }
    }
}
int check_guard_bytes(uint8_t* guards[4], int guard_length, int guard_val, const char* msg) {
    const char* names[4] = {"send preamble", "recv preamble", "send post", "recv post"};

    int rc = 0;
    for (int j=0; j<4; j++) {
    for (int k=0; k<guard_length; k++) {
        if (guards[j][k] != guard_val) {
            if (j<2) {
                printf("BUFFER PROBLEM: %s guard has been overwritten (value[-%d]=%d != %d)! (%s)\n",names[j],guard_length-k,guards[j][k],guard_val,msg);
            } else {
                printf("BUFFER PROBLEM: %s guard has been overwritten (value[+%d]=%d != %d)! (%s)\n",names[j],k,guards[j][k],guard_val,msg);
            }
            rc = 1;
        }
    }
    }
    return rc;
}
int execute_test(struct run_config *run) {

    int sdtype_size;
    int rdtype_size;
    int err;
    int sum_send_count;
    int sum_recv_count;
    int guard_len;
    guard_len = 30;

    uint8_t *svalidation_buf;
    uint8_t *rvalidation_buf;

    uint8_t *msg_guards[4];
    uint8_t *valb_guards[4];

    uint8_t *smsg_buf;
    uint8_t *rmsg_buf;
    int *sendcounts;
    int *recvcounts;
    int *sdispls;
    int *rdispls;

    int rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    err = MPI_Type_commit(&run->rdtype);
    ERROR_CHECK(err, on_error)
    err = MPI_Type_commit(&run->sdtype);
    ERROR_CHECK(err, on_error)

    MPI_Aint lbr, lbs, lbr_true, lbs_true;
    MPI_Aint lbs_shift, lbr_shift;
    MPI_Aint sdtype_extent, rdtype_extent;
    MPI_Aint sdtype_true_extent, rdtype_true_extent;
    MPI_Request req;
    MPI_Status status;

    MPI_Type_size(run->sdtype, &sdtype_size);
    MPI_Type_size(run->rdtype, &rdtype_size);
    if (sdtype_size * run->sdcount_mult != rdtype_size * run->rdcount_mult) {
        printf("Error in types or in test harness.  Attempting to send/recv types of differing sizes: %d*%d != %d*%d!\n",
            sdtype_size, run->sdcount_mult, rdtype_size, run->rdcount_mult);
        return 1;
    }

    MPI_Type_get_extent(run->sdtype, &lbs, &sdtype_extent);
    MPI_Type_get_extent(run->rdtype, &lbr, &rdtype_extent);
    MPI_Type_get_true_extent(run->rdtype, &lbr_true, &rdtype_true_extent);
    MPI_Type_get_true_extent(run->sdtype, &lbs_true, &sdtype_true_extent);
    if (VERBOSE_LEVEL_VERY_LOUD) {
        printf("Datatype      (send,recv) extents (%ld,%ld), size (%d,%d), and lb (%ld,%ld)\n",
            sdtype_extent, rdtype_extent, sdtype_size, rdtype_size, lbs, lbr);
        printf("Datatype TRUE (send,recv) extents (%ld,%ld), size (%d,%d), and lb (%ld,%ld)\n",
            sdtype_true_extent, rdtype_true_extent, sdtype_size, rdtype_size, lbs_true, lbr_true);
    }

    lbs_shift = -lbs_true;
    lbr_shift = -lbr_true;

    sum_send_count = run->sum_send_count * run->sdcount_mult;
    sum_recv_count = run->sum_recv_count * run->rdcount_mult;

    size_t send_buf_len, recv_buf_len;
    send_buf_len = (sum_send_count>0?1:0)*sdtype_true_extent + MAX(0,sum_send_count-1)*sdtype_extent;
    recv_buf_len = (sum_recv_count>0?1:0)*rdtype_true_extent + MAX(0,sum_recv_count-1)*rdtype_extent;
    // printf("SEND_BUF_LEN: Allocating (%ld)+(%ld) + 2*(%d) bytes\n",sdtype_true_extent,MAX(0,sum_send_count-1)*sdtype_extent, guard_len);

    svalidation_buf = (uint8_t*)malloc( sdtype_size * sum_send_count + 2*guard_len) + guard_len;
    rvalidation_buf = (uint8_t*)malloc( rdtype_size * sum_recv_count + 2*guard_len) + guard_len;
    valb_guards[0] = svalidation_buf -guard_len;
    valb_guards[1] = rvalidation_buf -guard_len;
    valb_guards[2] = svalidation_buf + sdtype_size * sum_send_count;
    valb_guards[3] = rvalidation_buf + rdtype_size * sum_recv_count;

    smsg_buf = (uint8_t*)malloc( send_buf_len + 2*guard_len) + guard_len;
    rmsg_buf = (uint8_t*)malloc( recv_buf_len + 2*guard_len) + guard_len;
    msg_guards[0] = smsg_buf - guard_len;
    msg_guards[1] = rmsg_buf - guard_len;
    msg_guards[2] = smsg_buf + send_buf_len;
    msg_guards[3] = rmsg_buf + recv_buf_len;

    set_guard_bytes(msg_guards, guard_len, 127);
    err = check_guard_bytes( msg_guards,  guard_len, 127, "SANITY1" );
    set_guard_bytes(valb_guards, guard_len, 128);
    err |= check_guard_bytes( valb_guards,  guard_len, 128, "SANITY2" );
    err |= check_guard_bytes( msg_guards,  guard_len, 127, "SANITY3" );
    ERROR_CHECK(err, on_error);

    sendcounts = (int*)malloc( sizeof(int) * world_size);
    recvcounts = (int*)malloc( sizeof(int) * world_size);
    sdispls    = (int*)malloc( sizeof(int) * world_size);
    rdispls    = (int*)malloc( sizeof(int) * world_size);
    for (int jrank = 0; jrank < world_size; jrank++ ) {
        sendcounts[jrank] = run->sendcounts[jrank] * run->sdcount_mult;
        recvcounts[jrank] = run->recvcounts[jrank] * run->rdcount_mult;
        sdispls[jrank] = run->sdispls[jrank] * run->sdcount_mult;
        rdispls[jrank] = run->rdispls[jrank] * run->rdcount_mult;
    }

    for (int jiter = 0; jiter < run->user->iters; jiter++) {

        fill_pattern_buf( svalidation_buf, sdtype_size * sum_send_count, rank, jiter);
        memset( smsg_buf, 0, send_buf_len);
        memset( rmsg_buf, 1, recv_buf_len);

        err = check_guard_bytes( msg_guards,  guard_len, 127, "message buffer1" );
        ERROR_CHECK(err, on_error);

        err = check_guard_bytes( valb_guards, guard_len, 128, "validation buffer" );
        ERROR_CHECK(err, on_error);


        /* move data from pattern buf to message buf using send-to-self calls: */
        err = MPI_Irecv(smsg_buf+lbs_shift, sum_send_count, run->sdtype, 0, 0, MPI_COMM_SELF, &req);
        ERROR_CHECK(err, on_error);
        err = MPI_Send(svalidation_buf,  sdtype_size * sum_send_count, MPI_BYTE, 0, 0, MPI_COMM_SELF);
        ERROR_CHECK(err, on_error);
        err = MPI_Wait(&req, &status);
        ERROR_CHECK(err==MPI_ERR_IN_STATUS && status.MPI_ERROR, on_error);
        err = check_guard_bytes( msg_guards,  guard_len, 127, "message buffer after filling send-buf with pattern" );
        err |= check_guard_bytes( valb_guards, guard_len, 128, "validation buffer after filling send-buf with pattern" );
        ERROR_CHECK(err, on_error);

        /* exchange data */
        err = MPI_Alltoallv(
            smsg_buf+lbs_shift, sendcounts, sdispls, run->sdtype,
            rmsg_buf+lbr_shift, recvcounts, rdispls, run->rdtype,
            MPI_COMM_WORLD
            );
        ERROR_CHECK(err, on_error);
        err = check_guard_bytes( msg_guards,  guard_len, 127, "message buffer3" );
        err |= check_guard_bytes( valb_guards, guard_len, 128, "validation buffer" );
        ERROR_CHECK(err, on_error);

        MPI_Barrier(MPI_COMM_WORLD);

        /* move data from rmsg_buf to receive validation buf using send-to-self calls: */
        err = MPI_Irecv(rvalidation_buf, rdtype_size * sum_recv_count, MPI_BYTE, 0, 0, MPI_COMM_SELF, &req);
        ERROR_CHECK(err, on_error);
        err = MPI_Send(rmsg_buf+lbr_shift, sum_recv_count, run->rdtype, 0, 0, MPI_COMM_SELF);
        ERROR_CHECK(err, on_error);
        err = MPI_Wait(&req, &status);
        ERROR_CHECK(err==MPI_ERR_IN_STATUS && status.MPI_ERROR, on_error);
        ERROR_CHECK(err, on_error);
        err = check_guard_bytes( msg_guards,  guard_len, 127, "message buffer4" );
        err |= check_guard_bytes( valb_guards, guard_len, 128, "validation buffer" );
        ERROR_CHECK(err, on_error);

        /*
         * because:
         *     rdtype_size * rdcount_mult <=> sdtype_size * sdcount_mult <=> remote's sdtype_size * sdcount_mult
         *     run->remote_sdispls is in units of the remote's [sdtype_size*sdcount_mult] Bytes
         *
         * So let the check function scale everything by type_size*mult, and
         * provide the run-> varaibles for recvcounts and remote_sdispls
         */
        err = check_pattern_buf( rvalidation_buf, rdtype_size*run->rdcount_mult, world_size, run->recvcounts, run->remote_sdispls, jiter);
        if (err) {
            printf("ERROR: Validation failed on rank %d!\n",rank);
            goto on_error;
        }
        tot_bytes_sent += sdtype_size * sum_send_count;
        tot_bytes_recv += rdtype_size * sum_recv_count;
    }

    free(smsg_buf-guard_len);
    free(rmsg_buf-guard_len);
    free(svalidation_buf-guard_len);
    free(rvalidation_buf-guard_len);
    free(sendcounts);
    free(recvcounts);
    free(rdispls);
    free(sdispls);
    tot_tests_exec++;

    return 0;

    on_error:
    MPI_Abort(MPI_COMM_WORLD, 1);
    return 1;
}

int main(int argc, char *argv[]) {
    typedef std::chrono::high_resolution_clock myclock;
    myclock::time_point beginning = myclock::now();

    int err;


    MPI_Init(&argc, &argv);
    int rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    struct run_config run;
    run.user = &user;


    static struct option long_options[] = {
        { "seed",       required_argument,  0, 's' },
        { "item-count", required_argument,  0, 'c' },
        { "prob-item",  required_argument,  0, 'i' },
        { "prob-rank",   required_argument, 0, 'r' },
        { "prob-world", required_argument,  0, 'w' },
        { "iters",      required_argument,  0, 't' },
        { "only",       required_argument,  0, 'o' },
        { "verbose",    required_argument,  0, 'v' },
        { "verbose-rank", required_argument, 0, 'z' },
        { "help",       no_argument,        0, 'h' }
    };

    int opt;
    int option_index;
    int verbose_rank = 0;
    user.verbose = 0;

    while (1)
    {
        char *s1, *s2;
        opt = getopt_long(argc, argv, "s:c:i:r:w:t:v:hz:", long_options, &option_index);
        if (opt == -1) break;
        switch(opt) {
        case 's':
            user.seed = atoi(optarg);
            break;
        case 'c':
            user.item_count = atoi(optarg);
            break;
        case 'i':
            user.prob_item = atof(optarg);
            break;
        case 'r':
            user.prob_rank = atof(optarg);
            break;
        case 'w':
            user.prob_world = atof(optarg);
            break;
        case 't':
            user.iters = atoi(optarg);
            break;
        case 'o':
            s1 = strtok(optarg, ",");
            s2 = strtok(NULL, ",");
            if (s1==NULL || s2==NULL) {
                print_help();
                printf("Option to --only should be like \"0,3\".");
            }
            user.only_high = atoi(s1);
            user.only_low = atoi(s2);
            break;
        case 'v':
            user.verbose = atoi(optarg);
            break;
        case 'h':
            if (rank==0) {
                print_help();
            }
            MPI_Finalize();
            return EXIT_SUCCESS;
        case 'z':
            verbose_rank = atoi(optarg);
            break;
        default:
            if (rank==0) {
                print_help();
                printf("Unexpected option: %c\n",opt);
            }
            MPI_Finalize();
            return EXIT_FAILURE;
        }
    }
    if (verbose_rank != -1 && verbose_rank != rank) {
        user.verbose = 0;
    }

    if (VERBOSE_LEVEL_DEFAULT && (user.only_high || user.only_low)) {
        printf("Requested only test %d,%d\n",user.only_high,user.only_low);
    }

    if (VERBOSE_LEVEL_LOUD) {
        dump_user_config(&user);
        printf("-----------\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);
    // mt19937 is a standard mersenne_twister_engine, seeded with our rank.
    std::mt19937 rngseq(user.seed+rank);
    std::uniform_real_distribution<double> uniform_double(0.0, 1.0);
    std::uniform_int_distribution<uint32_t> uniform_uint32(0, UINT32_MAX);
    // printf("On rank %d: Rank+seed seed produced: %u then %f\n",rank, uniform_uint32(rngseq), uniform_double(rngseq));

    uint8_t *send_mat   = (uint8_t*)malloc(sizeof(*send_mat) * world_size * user.item_count);
    uint8_t *recv_mat   = (uint8_t*)malloc(sizeof(*send_mat) * world_size * user.item_count);
    uint8_t *rank_on    = (uint8_t*)malloc(sizeof(*rank_on) * world_size);

    uint8_t this_rank_is_on = uniform_double(rngseq) < user.prob_world;

    err = MPI_Allgather( &this_rank_is_on, 1, MPI_UINT8_T, rank_on, 1, MPI_UINT8_T, MPI_COMM_WORLD);
    ERROR_CHECK( err, on_error );

    for (int jrank=0; jrank<world_size; jrank++) {
        bool rank_exchange_on = uniform_double(rngseq) < user.prob_rank;
        for (int jitem=0; jitem<user.item_count; jitem++) {
            bool item_exchange_on = uniform_double(rngseq) < user.prob_item;
            send_mat[ jitem + jrank*user.item_count ] = item_exchange_on && rank_exchange_on && rank_on[jrank] && rank_on[rank];
        }
    }

    err = MPI_Alltoall(send_mat, user.item_count, MPI_UINT8_T, recv_mat, user.item_count, MPI_UINT8_T, MPI_COMM_WORLD);
    ERROR_CHECK( err, on_error );


    if ( (VERBOSE_LEVEL_DEFAULT && user.item_count < 12) ) {
        if (!this_rank_is_on) {
            printf("Rank %d will sit out all transfers\n",rank);
        }

        for (int print_rank=0; print_rank < world_size; print_rank++) {
            if (rank==print_rank) {
                for (int jrank=0; jrank < world_size; jrank++) {
                    printf("%3d to %3d: ", rank, jrank);
                    for (int jitem=0; jitem<user.item_count; jitem++) {
                        printf("%1d",send_mat[ jitem + jrank*user.item_count ]);
                    }
                    printf(" [send]\n");
                }
            }
        }
    }


    run.sendcounts = (int*)malloc( sizeof(*run.sendcounts)*world_size);
    run.recvcounts = (int*)malloc( sizeof(*run.recvcounts)*world_size);
    run.sdispls = (int*)malloc( sizeof(*run.sdispls)*world_size);
    run.rdispls = (int*)malloc( sizeof(*run.rdispls)*world_size);
    run.remote_sdispls = (int*)malloc( sizeof(*run.remote_sdispls)*world_size);
    run.send_mat = send_mat;
    run.recv_mat = recv_mat;
    run.user = &user;


    run.sum_send_count = 0;
    run.sum_recv_count = 0;
    for (int jrank=0; jrank < world_size; jrank++) {
        run.sendcounts[jrank] = 0;
        run.recvcounts[jrank] = 0;
        run.sdispls[jrank] = run.sum_send_count;
        run.rdispls[jrank] = run.sum_recv_count;
        for (int jitem=0; jitem<user.item_count; jitem++) {
            run.sendcounts[jrank] += send_mat[ jitem + jrank*user.item_count ];
            run.recvcounts[jrank] += recv_mat[ jitem + jrank*user.item_count ];
            run.sum_send_count += send_mat[ jitem + jrank*user.item_count ];
            run.sum_recv_count += recv_mat[ jitem + jrank*user.item_count ];
        }
    }

    /* we need these for validation purposes */
    err = MPI_Alltoall(run.sdispls, 1, MPI_INT, run.remote_sdispls, 1, MPI_INT, MPI_COMM_WORLD);
    ERROR_CHECK( err, on_error );


    err = top_level_exhaustive(&run);
    ERROR_CHECK( err, on_error );



    MPI_Finalize();
    if (VERBOSE_LEVEL_LOUD || rank==verbose_rank) {
        printf("Rank %d sent %ld bytes, and received %ld bytes.\n",rank,tot_bytes_sent,tot_bytes_recv);
    }

    if (rank==verbose_rank) {
        printf("[OK] All tests passsed.  Executed %ld tests with seed %d with %d total ranks).\n",tot_tests_exec, user.seed, world_size);
    }

    return 0;

    on_error:
    printf("TEST FAILED.\n");
    return 1;
}