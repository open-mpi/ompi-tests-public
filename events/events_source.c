/*
 * events_source.c
 *
 * Test events source functions
 *  
 */

#include "events_common.h"


//  Test flags
int event_source_get_num_success = 0;
int event_source_get_info_success = 0;
int event_source_get_timestamp_success = 0;


void print_results() {

    if ( 0 != rank ) return;

    print_pf_result("MPI_T_source_get_num", "Get Number of Sources Success", event_source_get_num_success);
    print_pf_result("MPI_T_source_get_info", "Get Source Info Success", event_source_get_info_success);
    print_pf_result("MPI_T_source_get_timestamp", "Get Source Timestamp Success", event_source_get_timestamp_success);

    if ( do_failure_tests ) {
    }

    fprintf(outstream, "%-*s - %-*s : %6d\n", func_width, "TOTAL ERROR COUNT", metric_width, "", error_count);
}


void test_source() {

    int retval;

    int num_sources, source_index;
    MPI_Count timestamp;
    char name[MAX_STRING], desc[MAX_STRING];
    int name_len = MAX_STRING, desc_len = MAX_STRING;
    MPI_Count ticks_per_second, max_ticks;
    MPI_Info info;
    MPI_T_source_order ordering;

    /*
     *
     * int MPI_T_source_get_num(int *num_sources)
     * int MPI_T_source_get_info(int source_index, char *name, int *name_len,
     *               char *desc, int *desc_len, MPI_T_source_order *ordering,
     *               MPI_Count *ticks_per_second, MPI_Count *max_ticks,
     *               MPI_Info *info)
     * int MPI_T_source_get_timestamp(int source_index, MPI_Count *timestamp)
     */

    retval = MPI_T_source_get_num(&num_sources);

    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_source_get_num did not return MPI_SUCCESS", retval, TEST_CONTINUE);
    else 
        event_source_get_num_success = 1;


    print_debug("num_sources is %d\n", num_sources);

    if ( num_sources > 0 )
        source_index = 0;

    retval = MPI_T_source_get_info(source_index, name, &name_len,
            desc, &desc_len, &ordering, &ticks_per_second, &max_ticks, &info);

    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_source_get_info did not return MPI_SUCCESS", retval, TEST_CONTINUE);
    else {
            
        event_source_get_info_success = 1;

        print_debug("source %3d : %s : %s : %lu : %lu\n", source_index, name, desc, ticks_per_second, max_ticks);
    }


    retval = MPI_T_source_get_timestamp(source_index, &timestamp);

    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_source_get_timestamp did not return MPI_SUCCESS", retval, TEST_CONTINUE);
    else 
        event_source_get_timestamp_success = 1;


    if ( do_failure_tests ) {
    }

}


int main (int argc, char** argv)
{

    test_init("MPI_T Events Source Tests", argc, argv);

    test_source();

    print_results();

    MPI_T_finalize();
    MPI_Finalize();

    return 0;
}

