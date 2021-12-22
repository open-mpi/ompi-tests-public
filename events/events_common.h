/*
 * events_common.h
 *
 * Common functionality available for all events tests
 *  
 */

#if !defined INCLUDE_EVENT_COMMON
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#define MAX_STRING 4096

enum { TEST_CONTINUE = 0, TEST_EXIT = 1 };
typedef struct event_info_t {
        int event_index;
        char *name;
        int name_len;
        int verbosity;
        MPI_Datatype *array_of_datatypes;
        MPI_Aint *array_of_displacements;
        int num_elements;
        MPI_T_enum enumtype;
        MPI_Info info;
        char *desc;
        int desc_len;
        int bind;

} EVENT_INFO;

extern FILE* errout;
extern int error_count;
extern int print_events;
extern int debug_level;
extern int print_errors;
extern int do_failure_tests;
extern int event_index;
extern char *cb_user_data;
extern int rank;
extern int wsize;
extern FILE *outstream;
extern int func_width;
extern int metric_width;
extern char *pass_str;
extern char *fail_str;

enum { NO_MPI_ERROR_CODE = -1 };

extern char* bind_str(int b);
extern void print_error(char *errstr, int errcode, int exit_flag);
extern void print_debug(const char * format, ... );
extern void generate_callback_activity(void);
extern void generate_callback_activity_1proc(void);
extern void test_init(char *test_name, int ac, char**av);
extern void print_pf_result(char* function, char *testname, int flag);


#define INCLUDE_EVENT_COMMON
#endif
