/*
 * events_meta_data.c
 *
 * Test events metadata functions for timestamp and source
 *  
 */

#include "events_common.h"

char user_data_string[MAX_STRING] = "Test String";


//  Test flags
int event_read_time_success = 0;
int event_increasing_time_success = 0;
int event_get_source_success = 0;


void print_results() {

    if ( 0 == rank ) {
        print_pf_result("MPI_T_event_get_timestamp", "Successful return", event_read_time_success);
        print_pf_result("MPI_T_event_get_timestamp", "Increasing timestamps", event_increasing_time_success);
        print_pf_result("MPI_T_event_get_source", "Successful return", event_get_source_success);

        fprintf(outstream, "%-*s - %-*s : %6d\n", func_width, "TOTAL ERROR COUNT", metric_width, "", error_count);
    }
}


/*
 * typedef void (*MPI_T_event_cb_function) (MPI_T_event_instance event,
        MPI_T_event_registration handle,
        MPI_T_cb_safety cb_safety,
        void *user_data);
 */

void test_event_cb_function(MPI_T_event_instance event, MPI_T_event_registration handle, MPI_T_cb_safety cb_safety, void *user_data) {

    int retval;
    MPI_Count event_timestamp, previous_timestamp = 0;
    int source_index;

    print_debug("In cb_function : %s %s %d\n", __func__, __FILE__, __LINE__);

    retval = MPI_T_event_get_timestamp(event, &event_timestamp);

    if ( retval != MPI_SUCCESS ) {
        print_error("MPI_T_event_get_timestamp did not return MPI_SUCCESS", retval, TEST_CONTINUE);
        event_read_time_success = 0;
    }
    else {
        print_debug("MPI_T_event_get_timestamp provided MPI_Count %lu\n", event_timestamp);
        event_read_time_success = 1;
        previous_timestamp = event_timestamp;
    }

    if ( previous_timestamp > 0 && previous_timestamp <= event_timestamp )
        event_increasing_time_success = 1;

    retval = MPI_T_event_get_source(event, &source_index);

    if ( retval != MPI_SUCCESS ) {
        print_error("MPI_T_event_get_source did not return MPI_SUCCESS", retval, TEST_CONTINUE);
        event_get_source_success = 0;
    }
    else {
        print_debug("MPI_T_event_get_source provided source_index %d\n", source_index);
        event_get_source_success = 1;
    }
}


void test_free_event_cb_function(MPI_T_event_registration handle, MPI_T_cb_safety cb_safety, void *user_data) {

    print_debug("In cb_function : %s %s %d\n", __func__, __FILE__, __LINE__);
}


void test_meta_data() {
    int retval;
    int event_index;
    MPI_T_event_registration event_registration;
    char *event_name = "ompi_pml_ob1_request_complete";

    retval = MPI_T_event_get_index(event_name, &event_index);

    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_get_index did not return MPI_SUCCESS", retval, TEST_EXIT);

    retval = MPI_T_event_handle_alloc(event_index, MPI_COMM_WORLD, MPI_INFO_NULL, &event_registration) ;

    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_handle_alloc did not return MPI_SUCCESS", retval, TEST_EXIT);

    cb_user_data = user_data_string;
    print_debug("Testing expected MPI_T_event_register_callback success for index %d\n", event_index);

    retval = MPI_T_event_register_callback(event_registration, MPI_T_CB_REQUIRE_ASYNC_SIGNAL_SAFE, MPI_INFO_NULL, cb_user_data, test_event_cb_function);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_register_callback did not return MPI_SUCCESS", retval, TEST_EXIT);

    if ( do_failure_tests ) {
    }

    generate_callback_activity();
}


int main (int argc, char** argv)
{

    test_init("MPI_T Events Event Meta Data Tests", argc, argv);

    test_meta_data();

    print_results();

    MPI_T_finalize();
    MPI_Finalize();

    return 0;
}

