/*
 * events_dropped.c
 *
 * Test dropped event callback registration and execution
 *  
 */

#include "events_common.h"


char user_data_string[256] = "Test String";

//  Test flags
int event_cb_success = 0;
int event_free_cb_success = 0;
int event_free_cb_user_data_access = 1;
int event_handle_get_info_success = 1;
int event_handle_set_info_success = 1;
int event_handle_set_info_updated = 0;
int event_callback_get_info_success = 1;
int event_callback_set_info_success = 1;
int event_callback_set_info_updated = 0;
int event_handle_alloc_event_index_exceed_handled = 1;
int event_handle_alloc_event_index_negative_handled = 1;

int event_set_dropped_handler_success = 0;
int event_dropped_cb_success = 0;


void print_results() {

    if ( 0 != rank ) return;

    if ( 0 == event_dropped_cb_success ) 
        error_count++;

    print_pf_result("MPI_T_event_set_dropped_handler", "Event Set Dropped Callback Success", event_set_dropped_handler_success);
    print_pf_result("MPI_T_event_set_dropped_handler", "Event Set Dropped Callback Called", event_dropped_cb_success);

    fprintf(outstream, "%-*s - %-*s : %6d\n", func_width, "TOTAL ERROR COUNT", metric_width, "", error_count);

}


void test_event_dropped_cb_function(MPI_Count count, MPI_T_event_registration event_registration, int source_index, MPI_T_cb_safety cb_safety, void *user_data) {

    print_debug("In dropped_cb_function : %s %s %d\n", __func__, __FILE__, __LINE__);
    //print_debug("user_data is :%s:\n", (char*)user_data);
    //print_debug("user_data_string is :%s:\n", (char*)user_data_string);

    event_dropped_cb_success = 1;

    if ( NULL == user_data || strcmp(user_data, user_data_string) )
        print_error("test_event_cb_function could not access user_data", NO_MPI_ERROR_CODE, TEST_CONTINUE);
}


void test_dropped() {

    int i, retval;

    /*
     * int MPI_T_event_set_dropped_handler(
     *              MPI_T_event_registration event_registration,
     *              MPI_T_event_dropped_cb_function dropped_cb_function)
     *
     * typedef void MPI_T_event_dropped_cb_function(MPI_Count count, 
     *              MPI_T_event_registration event_registration, int source_index,
     *              MPI_T_cb_safety cb_safety, void *user_data);
     *
     */
    int event_index;
    MPI_T_event_registration event_registration;

    for ( i = 0; i < 1; i++ ) {
        event_index = i;

        print_debug("Testing expected success for index %d\n", event_index);
        retval = MPI_T_event_handle_alloc(event_index, MPI_COMM_WORLD, MPI_INFO_NULL, &event_registration) ;

        if ( retval != MPI_SUCCESS )
            print_error("MPI_T_event_handle_alloc did not return MPI_SUCCESS", retval, TEST_EXIT);

        /*
         * int MPI_T_event_set_dropped_handler(
         *              MPI_T_event_registration event_registration,
         *              MPI_T_event_dropped_cb_function dropped_cb_function)
         */

        cb_user_data = (void*)user_data_string;
        print_debug("cb_user_data is %s\n", cb_user_data);

        print_debug("Testing expected MPI_T_event_register_callback success for index %d\n", event_index);
        retval = MPI_T_event_set_dropped_handler(event_registration, test_event_dropped_cb_function);

        if ( retval != MPI_SUCCESS )
            print_error("MPI_T_event_set_dropped_handler did not return MPI_SUCCESS", retval, TEST_EXIT);
        else
            event_set_dropped_handler_success = 1;

        if ( do_failure_tests ) {
        }

        generate_callback_activity();
    }
}

int main (int argc, char** argv)
{

    test_init("MPI_T Events Callback Tests", argc, argv);

    test_dropped();

    print_results();

    MPI_T_finalize();
    MPI_Finalize();

    return 0;
}

