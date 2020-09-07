/*
 * Copyright (C) 2020 Cisco Systems, Inc.
 *
 * $COPYRIGHT$
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#include "mpi.h"


// Globals in this file, for convenience.
static int rank = -1;
static MPI_Fint f_mpi_status_size = -1;
static MPI_Fint f_mpi_source = -1;
static MPI_Fint f_mpi_tag = -1;
static MPI_Fint f_mpi_error = -1;


// Prototype the Fortran functions that we'll call from here in C
void check_cancelled_f_wrapper(MPI_Fint *f_status,
                               MPI_Fint *expected_value,
                               const char *msg);
void check_count_f(MPI_Fint *f_status, MPI_Fint *expected_tag,
                   const char *msg);
void check_cancelled_f08_wrapper(MPI_F08_status *f08_status,
                                 MPI_Fint *expected_value,
                                 const char *msg);
void check_count_f08(MPI_F08_status *f08_status, MPI_Fint *expected_tag,
                     const char *msg);

/////////////////////////////////////////////////////////////////////////

// The first argument is either an MPI_Fint or something that can be
// upcasted to an MPI_Fint (i.e., a C int, but sizeof(int)==4 and
// sizeof(MPI_Fint)==8).
static void check_int(MPI_Fint fint, int cint, const char *msg)
{
    if (fint != cint) {
        printf("Error: %s: %d != %d\n", msg, fint, cint);
        exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////

static void check_cancelled_c(MPI_Status *status, int expected_value,
                              const char *msg)
{
    int value;
    MPI_Test_cancelled(status, &value);
    if (value != expected_value) {
        printf("Error: %s: %d != %d\n", msg, value, expected_value);
        exit(1);
    }
}

static void check_count_c(MPI_Status *status, MPI_Datatype type,
                          int expected_count, const char *msg)
{
    int count;
    MPI_Get_count(status, MPI_INTEGER, &count);
    check_int(count, expected_count, msg);
}

/////////////////////////////////////////////////////////////////////////

static void test_f082c(MPI_F08_status *f08_status, MPI_Fint f08_tag,
                MPI_Fint expected_count)
{
    MPI_Status c_status;

    printf("Testing C MPI_Status_f082c\n");

    // NOTE: The f08_status has "cancelled" set to true.
    MPI_Status_f082c(f08_status, &c_status);

    check_int(c_status.MPI_SOURCE, rank, "f082c source");
    check_int(c_status.MPI_TAG, (int) f08_tag, "f082c tag");
    check_int(c_status.MPI_ERROR, MPI_SUCCESS, "f082c success");
    check_cancelled_c(&c_status, 1, "f082c cancelled=.true.");
    check_count_c(&c_status, MPI_INTEGER, (int) expected_count,
                  "f082c count");
}

/////////////////////////////////////////////////////////////////////////

static void test_c2f08(MPI_Status *c_status, int c_tag,
                       int expected_count)
{
    MPI_Fint temp;
    MPI_F08_status f08_status;

    printf("Testing C MPI_Status_c2f08\n");

    // First test with cancelled=0
    MPI_Status_set_cancelled(c_status, 0);
    MPI_Status_c2f08(c_status, &f08_status);

    check_int(f08_status.MPI_SOURCE, rank, "c2f08 source");
    check_int(f08_status.MPI_TAG, c_tag, "c2f08 tag");
    check_int(f08_status.MPI_ERROR, MPI_SUCCESS, "c2f08 success");
    temp = 0;
    check_cancelled_f08_wrapper(&f08_status, &temp, "c2f08 cancelled=0");
    temp = (MPI_Fint) expected_count;
    check_count_f08(&f08_status, &temp, "c2f08 count");

    // Then test with cancelled=1
    MPI_Status_set_cancelled(c_status, 1);
    MPI_Status_c2f08(c_status, &f08_status);

    check_int(f08_status.MPI_SOURCE, rank, "c2f08 source");
    check_int(f08_status.MPI_TAG, c_tag, "c2f08 tag");
    check_int(f08_status.MPI_ERROR, MPI_SUCCESS, "c2f08 success");
    temp = (MPI_Fint) 1;
    check_cancelled_f08_wrapper(&f08_status, &temp, "c2f08 cancelled=1");
    temp = (MPI_Fint) expected_count;
    check_count_f08(&f08_status, &temp, "c2f08 count");
}

/////////////////////////////////////////////////////////////////////////

static void test_f2c(MPI_Fint *f_status, MPI_Fint f_tag,
                     MPI_Fint expected_count)
{
    MPI_Status c_status;

    printf("Testing C MPI_Status_f2c\n");

    // NOTE: The f_status has "cancelled" set to true.
    MPI_Status_f2c(f_status, &c_status);

    check_int(c_status.MPI_SOURCE, rank, "f2c source");
    check_int(c_status.MPI_TAG, (int) f_tag, "f2c tag");
    check_int(c_status.MPI_ERROR, MPI_SUCCESS, "f2c success");
    check_cancelled_c(&c_status, 1, "f2c cancelled=.true.");
    check_count_c(&c_status, MPI_INTEGER, (int) expected_count,
                  "f2c count");
}

/////////////////////////////////////////////////////////////////////////

static void test_c2f(MPI_Status *c_status, int c_tag,
                     int expected_count)
{
    MPI_Fint temp;
    MPI_Fint *f_status = alloca((size_t) f_mpi_status_size * sizeof(MPI_Fint));

    printf("Testing C MPI_Status_c2f\n");

    // First test with cancelled=0
    MPI_Status_set_cancelled(c_status, 0);
    MPI_Status_c2f(c_status, f_status);

    check_int(f_status[f_mpi_source], rank, "c2f source");
    check_int(f_status[f_mpi_tag], c_tag, "c2f tag");
    check_int(f_status[f_mpi_error], MPI_SUCCESS, "c2f success");
    temp = 0;
    check_cancelled_f_wrapper(f_status, &temp, "c2f cancelled=0");
    temp = (MPI_Fint) expected_count;
    check_count_f(f_status, &temp, "c2f count");

    // Then test with cancelled=1
    MPI_Status_set_cancelled(c_status, 1);
    MPI_Status_c2f(c_status, f_status);

    check_int(f_status[f_mpi_source], rank, "c2f source 2");
    check_int(f_status[f_mpi_tag], c_tag, "c2f tag 2");
    check_int(f_status[f_mpi_error], MPI_SUCCESS, "c2f success 2");
    temp = (MPI_Fint) 1;
    check_cancelled_f_wrapper(f_status, &temp, "c2f cancelled=1");
    temp = (MPI_Fint) expected_count;
    check_count_f(f_status, &temp, "c2f count 2");
}

/////////////////////////////////////////////////////////////////////////

static void test_f082f(MPI_F08_status *f08_status, MPI_Fint f08_tag,
                       MPI_Fint expected_count)
{
    MPI_Fint temp;
    MPI_Fint *f_status = alloca((size_t) f_mpi_status_size * sizeof(MPI_Fint));

    printf("Testing C MPI_Status_f082f\n");

    // NOTE: The f08_status has "cancelled" set to true.
    MPI_Status_f082f(f08_status, f_status);

    check_int(f_status[f_mpi_source], rank, "f082f source");
    check_int(f_status[f_mpi_tag], f08_tag, "f082f tag");
    check_int(f_status[f_mpi_error], MPI_SUCCESS, "f082f success");
    temp = 1;
    check_cancelled_f_wrapper(f_status, &temp, "f082f cancelled=.true.");
    check_count_f(f_status, &expected_count, "f082f count");
}

/////////////////////////////////////////////////////////////////////////

static void test_f2f08(MPI_Fint *f_status, MPI_Fint f_tag,
                       MPI_Fint expected_count)
{
    MPI_Fint temp;
    MPI_F08_status f08_status;

    printf("Testing C MPI_Status_f2f08\n");

    // NOTE: The f_status has "cancelled" set to true.
    MPI_Status_f2f08(f_status, &f08_status);

    check_int(f08_status.MPI_SOURCE, rank, "f2f08 source");
    check_int(f08_status.MPI_TAG, f_tag, "f2f08 tag");
    check_int(f08_status.MPI_ERROR, MPI_SUCCESS, "f2f08 success");
    temp = 1;
    check_cancelled_f_wrapper(f_status, &temp, "f2f08 cancelled=.true.");
    check_count_f(f_status, &expected_count, "f2f08 count");
}

/////////////////////////////////////////////////////////////////////////

static void generate_c_status(MPI_Status *status, int c_tag, int c_count)
{
    MPI_Fint sendbuf, recvbuf;
    MPI_Request requests[2];
    MPI_Status statuses[2];

    // Create a status the "normal" way
    //
    // Use a non-blocking send/receive to ourselves so that we can use
    // an array WAIT function so that the MPI_ERROR element will be
    // set in the resulting status.

    sendbuf = 789;
    MPI_Irecv(&recvbuf, 1, MPI_INTEGER, rank, c_tag, MPI_COMM_WORLD,
              &requests[0]);
    MPI_Isend(&sendbuf, 1, MPI_INTEGER, rank, c_tag, MPI_COMM_WORLD,
              &requests[1]);
    MPI_Waitall(2, requests, statuses);

    // Copy the resulting receive status to our output status
    memcpy(status, &statuses[0], sizeof(MPI_Status));

    // Now set some slightly different values in the status that
    // results in a very large count (larger than can be represented
    // by 32 bits)
    MPI_Status_set_cancelled(status, 0);
    MPI_Status_set_elements(status, MPI_INTEGER, c_count);
}

/////////////////////////////////////////////////////////////////////////

// This function is the entry point from Fortran
void test_c_functions(MPI_F08_status *f08_status, MPI_Fint *f08_tag,
                      MPI_Fint *f_status, MPI_Fint *f_tag, MPI_Fint *f_count,
                      MPI_Fint *mpi_status_size, MPI_Fint *mpi_source,
                      MPI_Fint *mpi_tag, MPI_Fint *mpi_error)
{
    MPI_Status c_status;
    int c_tag;
    int c_count;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // MPI (up to MPI-4, at least) specifically defines that
    // MPI_STATUS_SIZE is only available in Fortran.  So we need to
    // pass it in from Fortran so that we can have it here in C.
    // Ditto for MPI_SOURCE, MPI_TAG, and MPI_ERROR.  But note that
    // those values are indexes in to an array, and since Fortran
    // indexes on 1 and C indexes on 0, subtract 1 from those values
    // here in C.
    f_mpi_status_size = *mpi_status_size;
    f_mpi_source      = *mpi_source - 1;
    f_mpi_tag         = *mpi_tag - 1;
    f_mpi_error       = *mpi_error - 1;

    printf("Testing C functions...\n");

    // Make the C values slightly different than the Fortran values
    c_tag = 333;
    c_count = ((int) *f_count) + 99;

    generate_c_status(&c_status, c_tag, c_count);

    test_f082c(f08_status, *f08_tag, *f_count);
    test_c2f08(&c_status, c_tag, c_count);

    test_f2c(f_status, *f_tag, *f_count);
    test_c2f(&c_status, c_tag, c_count);

    test_f082f(f08_status, *f08_tag, *f_count);
    test_f2f08(f_status, *f_tag, *f_count);
}
