/*
 * events_callbacks.c
 *
 * Test event callback registration and free
 * Test handle and callback get/set info
 *  
 */

#include "events_common.h"

char user_data_string[MAX_STRING] = "Test String";

// Test flags
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


void print_results() {

    if ( 0 != rank ) return;

    print_pf_result("MPI_T_event_register_callback", "Event Callback Success", event_cb_success);
    print_pf_result("MPI_T_event_handle_free", "Event Free Callback Success", event_free_cb_success);
    print_pf_result("MPI_T_event_handle_free", "Event Free user_data access", event_free_cb_user_data_access);
    print_pf_result("MPI_T_event_handle_get_info", "Successful call", event_handle_get_info_success);
    print_pf_result("MPI_T_event_handle_set_info", "Successful call", event_handle_set_info_success);
    print_pf_result("MPI_T_event_handle_set_info", "Key added to Info object", event_handle_set_info_updated);
    print_pf_result("MPI_T_event_callback_get_info", "Successful call", event_callback_get_info_success);
    print_pf_result("MPI_T_event_callback_set_info", "Successful call", event_callback_set_info_success);
    print_pf_result("MPI_T_event_callback_set_info", "Key added to Info object", event_callback_set_info_updated);

    if ( do_failure_tests ) {
        print_pf_result("MPI_T_event_handle_alloc", "Handled event index too large", event_handle_alloc_event_index_exceed_handled);
        print_pf_result("MPI_T_event_handle_alloc", "Handled negative event index", event_handle_alloc_event_index_negative_handled);
    }

    fprintf(outstream, "%-*s - %-*s : %6d\n", func_width, "TOTAL ERROR COUNT", metric_width, "", error_count);

}


void test_event_cb_function(MPI_T_event_instance event, MPI_T_event_registration handle, MPI_T_cb_safety cb_safety, void *user_data) {

    print_debug("In cb_function : %s %s %d\n", __func__, __FILE__, __LINE__);
    //print_debug("user_data is :%s:\n", (char*)user_data);
    //print_debug("user_data_string is :%s:\n", (char*)user_data_string);

    event_cb_success = 1;

    if ( NULL == user_data || strcmp(user_data, user_data_string) )
        print_error("test_event_cb_function could not access user_data", NO_MPI_ERROR_CODE, TEST_CONTINUE);
}


void test_free_event_cb_function(MPI_T_event_registration handle, MPI_T_cb_safety cb_safety, void *user_data) {

    print_debug("In cb_function : %s %s %d\n", __func__, __FILE__, __LINE__);
    print_debug("user_data is :%s:\n", (char*)user_data);
    event_free_cb_success = 1;
    if ( NULL == user_data || strcmp(user_data, user_data_string) ) {
        print_error("test_event_free_cb_function could not access user_data", NO_MPI_ERROR_CODE, TEST_CONTINUE);
        event_free_cb_user_data_access = 0;
    }
}


void test_handle_alloc_register_free() {

    int retval;

    MPI_T_event_registration event_registration;

    // TEST MPI_T_event_handle_alloc
    //

    print_debug("Testing expected success for index %d\n", event_index);
    retval = MPI_T_event_handle_alloc(event_index, MPI_COMM_WORLD, MPI_INFO_NULL, &event_registration) ;

    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_handle_alloc did not return MPI_SUCCESS", retval, TEST_EXIT);


    //  Test MPI_T_event_handle_alloc invalid arguments

    if ( do_failure_tests ) {

        int bad_event_index;
        MPI_T_event_registration bad_event_registration;

        bad_event_index = 11111111;
        print_debug("Testing expected failure for index %d\n", bad_event_index);
    
        retval = MPI_T_event_handle_alloc(bad_event_index, MPI_COMM_WORLD, MPI_INFO_NULL, &bad_event_registration) ;
        if ( retval == MPI_SUCCESS ) {
            print_error("MPI_T_event_handle_alloc returned MPI_SUCCESS with too large index", retval, TEST_EXIT);
            event_handle_alloc_event_index_exceed_handled = 0;
        }
        else
            print_debug("MPI_T_event_handle_alloc handled too large index\n", retval, TEST_EXIT);

        bad_event_index = -1;
        print_debug("Testing expected failure for index %d\n", bad_event_index);
   
        retval = MPI_T_event_handle_alloc(bad_event_index, MPI_COMM_WORLD, MPI_INFO_NULL, &bad_event_registration) ;
        if ( retval == MPI_SUCCESS ) {
            print_error("MPI_T_event_handle_alloc returned MPI_SUCCESS with negative index", retval, TEST_EXIT);
            event_handle_alloc_event_index_negative_handled = 0;
        }
        else
            print_debug("MPI_T_event_handle_alloc handled negative index\n", retval, TEST_EXIT);
    }


    // TEST MPI_T_event_register_callback
    //
    cb_user_data = (void*)user_data_string;
    print_debug("cb_user_data is %s\n", cb_user_data);

    print_debug("Testing expected MPI_T_event_register_callback success for index %d\n", event_index);
    retval = MPI_T_event_register_callback(event_registration, MPI_T_CB_REQUIRE_ASYNC_SIGNAL_SAFE, MPI_INFO_NULL, cb_user_data, test_event_cb_function);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_register_callback did not return MPI_SUCCESS", retval, TEST_EXIT);

    if ( do_failure_tests ) {
    }

    generate_callback_activity();
    
    //}

    // TEST MPI_T_event_handle_free
    //

    retval = MPI_T_event_handle_free(event_registration, cb_user_data, test_free_event_cb_function);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_handle_free did not return MPI_SUCCESS", retval, TEST_EXIT);

    if ( do_failure_tests ) {
    }

}

void print_info_obj(MPI_Info info) {

    int i, retval, vallen = MPI_MAX_INFO_VAL, nkeys, flag;
    char key[MPI_MAX_INFO_KEY];
    char value[MPI_MAX_INFO_VAL+1];

    retval = MPI_Info_get_nkeys(info, &nkeys);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_Info_get_nkeys did not return MPI_SUCCESS", retval, TEST_CONTINUE);

    print_debug("MPI_Info_get_nkeys nkeys is %d\n", nkeys);

    for ( i = 0; i < nkeys; i++ ) {

        MPI_Info_get_nthkey( info, i, key );
        retval = MPI_Info_get( info, key, vallen, value, &flag );
        if ( retval != MPI_SUCCESS ) {
            print_error("MPI_Info_get did not return MPI_SUCCESS", retval, TEST_CONTINUE);
        }
        else
            print_debug("Info entry index %2d -  %d:%s:%s:%d\n", i, flag, key, value);
    }

}


void test_info() {

    int retval, vallen = MPI_MAX_INFO_VAL, nkeys, flag, initial_nkeys = 0;
    MPI_T_event_registration event_registration;
    char key[MPI_MAX_INFO_KEY];
    char value[MPI_MAX_INFO_VAL+1];
    MPI_Info info;
    MPI_T_cb_safety cb_safety = MPI_T_CB_REQUIRE_ASYNC_SIGNAL_SAFE;

    //
    //  Tests
    //
    //  - Test for successful calls to all get/set info
    //  - Check to see if set info calls add values for get calls
    //     - Record initial nkeys count
    //     - Add unique key
    //     - get_info
    //     - See if nkeys has increased


    //
    //  Set Up Test
    //
    //  Allocate a handle to use for testing
    retval = MPI_T_event_handle_alloc(event_index, MPI_COMM_WORLD, MPI_INFO_NULL, &event_registration) ;

    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_handle_alloc did not return MPI_SUCCESS", retval, TEST_EXIT);


    //
    //  Get Handle Info
    //
    retval = MPI_T_event_handle_get_info(event_registration, &info);

    if ( retval != MPI_SUCCESS ) {
        print_error("MPI_T_event_handle_get_info did not return MPI_SUCCESS", retval, TEST_CONTINUE);
        event_handle_get_info_success = 0;
    }


    // 
    // Get And Store Handle Info Key Count
    // 
    retval = MPI_Info_get_nkeys(info, &nkeys);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_Info_get_nkeys did not return MPI_SUCCESS", retval, TEST_CONTINUE);
    else
        initial_nkeys = nkeys;

    print_debug("MPI_Info_get_nkeys nkeys is %d\n", nkeys);


    // 
    // Add Entry To Handle Info 
    // 
    retval = MPI_Info_set(info, "randomkey", "randomkeyvalue");
    if ( retval != MPI_SUCCESS )
        print_error("MPI_info_set did not return MPI_SUCCESS", retval, TEST_CONTINUE);

    print_info_obj(info);

    retval = MPI_Info_get_nkeys(info, &nkeys);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_Info_get_nkeys did not return MPI_SUCCESS", retval, TEST_CONTINUE);

    print_debug("After MPI_Info_set, MPI_Info_get_nkeys nkeys is %d\n", nkeys);

        
    // 
    // Confirm Entry Has Been Added
    // 
    MPI_Info_get_nthkey( info, 0, key );
    retval = MPI_Info_get( info, key, vallen, value, &flag );
    if ( retval != MPI_SUCCESS ) {
        print_error("MPI_Info_get did not return MPI_SUCCESS", retval, TEST_CONTINUE);
    }
    else
        print_debug("Verifying that info values are %d:%s:%s:%d\n", flag, key, value);


    // 
    // Set Handle Info
    // 
    retval = MPI_T_event_handle_set_info(event_registration, info);
    if ( retval != MPI_SUCCESS ) {
        print_error("MPI_T_event_handle_set_info did not return MPI_SUCCESS", retval, TEST_CONTINUE);
        event_handle_set_info_success = 0;
    }

    // 
    // Get Event Handle Info
    // 
    retval = MPI_T_event_handle_get_info(event_registration, &info);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_handle_get_info did not return MPI_SUCCESS", retval, TEST_CONTINUE);

    //
    // Test For Increase In Nkeys To Confirm Update
    //
    retval = MPI_Info_get_nkeys(info, &nkeys);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_Info_get_nkeys did not return MPI_SUCCESS", retval, TEST_CONTINUE);

    if ( nkeys > initial_nkeys )
        event_handle_set_info_updated = 1;
    else
        print_error("MPI_T_event_handle_set_info did not update info object", NO_MPI_ERROR_CODE, TEST_CONTINUE);

    print_debug("MPI_T_event_handle_get_info nkeys is %d\n", nkeys);



    // Register Callback
    retval = MPI_T_event_register_callback(event_registration, cb_safety, MPI_INFO_NULL, cb_user_data, test_event_cb_function);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_register_callback did not return MPI_SUCCESS", retval, TEST_EXIT);

    //
    //  Get Handle Info
    //
    retval = MPI_T_event_callback_get_info(event_registration, cb_safety, &info);

    if ( retval != MPI_SUCCESS ) {
        print_error("MPI_T_event_callback_get_info did not return MPI_SUCCESS", retval, TEST_CONTINUE);
        event_callback_get_info_success = 0;
    }


    // 
    // Get And Store Callback Info Key Count
    // 
    retval = MPI_Info_get_nkeys(info, &nkeys);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_Info_get_nkeys did not return MPI_SUCCESS", retval, TEST_CONTINUE);
    else
        initial_nkeys = nkeys;

    print_debug("MPI_Info_get_nkeys nkeys is %d\n", nkeys);


    // 
    // Add Entry To Callback Info 
    // 
    retval = MPI_Info_set(info, "randomkey", "randomkeyvalue");
    if ( retval != MPI_SUCCESS )
        print_error("MPI_info_set did not return MPI_SUCCESS", retval, TEST_CONTINUE);

    print_info_obj(info);

    retval = MPI_Info_get_nkeys(info, &nkeys);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_Info_get_nkeys did not return MPI_SUCCESS", retval, TEST_CONTINUE);

    print_debug("After MPI_Info_set, MPI_Info_get_nkeys nkeys is %d\n", nkeys);

        
    // 
    // Confirm Entry Has Been Added
    // 
    MPI_Info_get_nthkey( info, 0, key );
    retval = MPI_Info_get( info, key, vallen, value, &flag );
    if ( retval != MPI_SUCCESS ) {
        print_error("MPI_Info_get did not return MPI_SUCCESS", retval, TEST_CONTINUE);
    }
    else
        print_debug("Verifying that info values are %d:%s:%s:%d\n", flag, key, value);


    // 
    // Set Callback Info
    // 
    retval = MPI_T_event_callback_set_info(event_registration, cb_safety, info);
    if ( retval != MPI_SUCCESS ) {
        print_error("MPI_T_event_callback_set_info did not return MPI_SUCCESS", retval, TEST_CONTINUE);
        event_callback_set_info_success = 0;
    }

    // 
    // Get Event Callback Info
    // 
    retval = MPI_T_event_callback_get_info(event_registration, cb_safety, &info);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_callback_get_info did not return MPI_SUCCESS", retval, TEST_CONTINUE);

    //
    // Test For Increase In Nkeys To Confirm Update
    //
    retval = MPI_Info_get_nkeys(info, &nkeys);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_Info_get_nkeys did not return MPI_SUCCESS", retval, TEST_CONTINUE);

    if ( nkeys > initial_nkeys )
        event_callback_set_info_updated = 1;
    else
        print_error("MPI_T_event_callback_set_info did not update info object", NO_MPI_ERROR_CODE, TEST_CONTINUE);

    print_debug("MPI_T_event_callback_get_info nkeys is %d\n", nkeys);
}


int main (int argc, char** argv)
{

    test_init("MPI_T Events Callback Tests", argc, argv);

    test_handle_alloc_register_free();
    
    test_info();

    print_results();

    MPI_T_finalize();
    MPI_Finalize();

    return 0;
}

