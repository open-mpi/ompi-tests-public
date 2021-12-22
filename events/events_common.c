/*
 * events_common.c
 *
 * Common functionality available for all events tests
 *  
 */

#include "events_common.h"
#include <getopt.h>


FILE* errout;
int error_count = 0;
int print_events = 0;
int print_errors = 0;
int debug_level = 0;
int do_failure_tests = 0;
int event_index = 0;
char *cb_user_data;
int rank, wsize;
FILE *outstream;
int func_width = 35;
int metric_width = 35;
char *pass_str = "PASS";
char *fail_str = "FAIL";


char* bind_str(int b)
{
    
    switch(b) {
        case MPI_T_BIND_NO_OBJECT:
            return "No Binding";
            break;
        case MPI_T_BIND_MPI_COMM:
            return "Communicator Binding";
            break;
        case MPI_T_BIND_MPI_DATATYPE:
            return "Datatype Binding";
            break;
        case MPI_T_BIND_MPI_ERRHANDLER:
            return "Errorhandler Binding";
            break;
        case MPI_T_BIND_MPI_FILE:
            return "File Binding";
            break;
        case MPI_T_BIND_MPI_GROUP:
            return "Group Binding";
            break;
        case MPI_T_BIND_MPI_OP:
            return "Reduction Op Binding";
            break;
        case MPI_T_BIND_MPI_REQUEST:
            return "Request Binding";
            break;
        case MPI_T_BIND_MPI_WIN:
            return "Windows Binding";
            break;
        case MPI_T_BIND_MPI_MESSAGE:
            return "Message Binding";
            break;
        case MPI_T_BIND_MPI_INFO:
            return "Info Binding";
            break;
    }

    return "Undefined binding";
}


void generate_callback_activity() {

#define GENERATE_BUFFER_SIZE 32

    char buf[GENERATE_BUFFER_SIZE];
    int i, lim = 4;
    MPI_Status stat;
    MPI_Request req;

    print_debug("In %s %s %d\n", __func__, __FILE__, __LINE__);


    if ( wsize < 2 ) {
        print_error("Need at least 2 MPI processes to generate activity", NO_MPI_ERROR_CODE, TEST_CONTINUE);
        return;
    }
        
    for ( i = 0; i < lim; i++ ) {
        if ( rank == 0 ) {
            strcpy(cb_user_data, "Irecv");
            MPI_Irecv(buf, GENERATE_BUFFER_SIZE, MPI_CHAR, 1, 27, MPI_COMM_WORLD, &req);
        }
        else {
            strcpy(buf, "cat");
            strcpy(cb_user_data, "Isend");
            MPI_Isend(buf, GENERATE_BUFFER_SIZE, MPI_CHAR, 0, 27, MPI_COMM_WORLD, &req);
        }

        strcpy(cb_user_data, "Wait");
        MPI_Wait(&req, &stat);

        strcpy(cb_user_data, "Barrier");
        MPI_Barrier(MPI_COMM_WORLD);
    }
}


void print_error(char *errstr, int errcode, int exit_flag) {

    FILE* errout = stderr;
    char mpi_err_msg[MPI_MAX_ERROR_STRING];
    int mpi_err_msg_len = MPI_MAX_ERROR_STRING - 1;

    if ( rank > 0 ) return;

    error_count++;

    if ( print_errors ) {
        if ( errcode != NO_MPI_ERROR_CODE ) {
            if ( errcode != -18 ) {
                MPI_Error_string(errcode, mpi_err_msg, &mpi_err_msg_len);
            }
            else {
                strcpy(mpi_err_msg, "Encountered error with code -18");
            }

            fprintf(errout, "*** ERROR: %s - %s\n", errstr, mpi_err_msg);
        }
        else {
            fprintf(errout, "*** ERROR: %s\n", errstr);
        }
    }

    if ( 0 != exit_flag ) {

        fprintf(errout, "Exiting.\n");
        exit(-1);
    }
}



void print_debug(const char * format, ... ) {
    va_list args;

    va_start (args, format);

    if ( rank > 0 ) return;
    if ( debug_level ) {
        fprintf (errout, "DEBUG: ");
        vfprintf (errout, format, args);
    }

    va_end (args);
}

void get_options(int ac, char **av) {

    int c;

    while (1)
    {
        static struct option long_options[] =
        {
            {"print-debug",  no_argument,           0, 'd'},
            {"print-errors",  no_argument,          0, 'e'},
            {"do-failure-tests",  no_argument,      0, 'f'},
            {"event-index",     required_argument,  0, 'i'},
            {"print-events",     no_argument,       0, 'l'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (ac, av, "defi:l", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            case 'd':
                debug_level = 1;
                break;

            case 'e':
                print_errors = 1;
                break;

            case 'f':
                do_failure_tests = 1;
                break;

            case 'i':
                event_index = atoi(optarg);
                break;

            case 'l':
                print_events = 1;
                break;

            case '?':
                break;

            default:
                break;
        }
    }
}


void test_init(char *test_name, int ac, char**av) {

    int retval;

    errout = stderr;
    outstream = stdout;

    retval = MPI_Init(&ac, &av);

    if (MPI_SUCCESS != retval) {
        print_error("Failed to initialize MPI", retval, TEST_EXIT);
    }

    get_options(ac, av);

    MPI_Comm_size(MPI_COMM_WORLD, &wsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int required = MPI_THREAD_SINGLE;
    int provided = 0;

    retval = MPI_T_init_thread(required, &provided);

    if (MPI_SUCCESS != retval) {
        print_error("Failed to initialize MPI tool interface", retval, TEST_EXIT);
    }

    if ( 0 == rank ) {
        fprintf(outstream, "\n\n%s\n\n", test_name);
    }
}


void print_pf_result(char* function, char *testname, int flag) {
    fprintf(outstream, "%-*s - %-*s : %6s\n", func_width, function, metric_width, testname, flag ? pass_str : fail_str);
}

