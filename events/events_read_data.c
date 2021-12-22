/*
 * events_read_data.c
 *
 * Test events data access functions for success and behavior
 *  
 */

#include "events_common.h"

#define ELEMENT_BUFFER_SIZE 256


char user_data_string[MAX_STRING] = "Test String";

//  Test flags
int event_read_data_success = 0;
int event_read_data_confirm = 0;
int event_copy_data_success = 1;
int event_copy_data_match = 1;
int event_element_index_exceed_handled = 0;
int event_element_index_negative_handled = 0;


void print_results() {

    if ( 0 != rank ) return;

    print_pf_result("MPI_T_event_read", "Event Read Data Success", event_read_data_success);
    print_pf_result("MPI_T_event_read", "Event Read Data Confirm", event_read_data_confirm);
    print_pf_result("MPI_T_event_copy", "Event Copy Data Success", event_copy_data_success);
    print_pf_result("MPI_T_event_copy", "Event Copy Data Verified", event_copy_data_match);

    if ( do_failure_tests ) {
        print_pf_result("MPI_T_event_read", "Handled element index too large", event_element_index_exceed_handled);
        print_pf_result("MPI_T_event_read", "Handled negative element index", event_element_index_negative_handled);
    }

    fprintf(outstream, "%-*s - %-*s : %6d\n", func_width, "TOTAL ERROR COUNT", metric_width, "", error_count);
}


/*
 * typedef void (*MPI_T_event_cb_function) (MPI_T_event_instance event,
        MPI_T_event_registration handle,
        MPI_T_cb_safety cb_safety,
        void *user_data);
 */

void test_event_cb_function(MPI_T_event_instance event, MPI_T_event_registration handle, MPI_T_cb_safety cb_safety, void *user_data) {

    int i, retval, num_elements, event_idx = 0;
    void *element_buffer, *element_test_buffer;
    int element_buffer_length;
    MPI_Datatype *array_of_datatypes;
    MPI_Aint *array_of_displacements;
    char type_name[MPI_MAX_OBJECT_NAME];
    int resultlen, type_size;
    static int seq_num = 1;

    print_debug("In cb_function : %s %s %d\n", __func__, __FILE__, __LINE__);

    if ( rank != 0 ) return;

    /*  Get data element count */
    retval = MPI_T_event_get_info (
            event_idx, 
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            &num_elements,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
            );

    if (MPI_SUCCESS != retval) {
        print_error("MPI_T_event_get_info failed", NO_MPI_ERROR_CODE, TEST_CONTINUE);
    } 

    array_of_datatypes = (MPI_Datatype*)malloc(sizeof(MPI_Datatype)*num_elements);
    array_of_displacements = (MPI_Aint*)malloc(sizeof(MPI_Aint)*num_elements);

    retval = MPI_T_event_get_info (
            event_idx, 
            NULL,
            NULL,
            NULL,
            array_of_datatypes,
            array_of_displacements,
            &num_elements,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
            );

    element_buffer = malloc(ELEMENT_BUFFER_SIZE);
    element_test_buffer = malloc(ELEMENT_BUFFER_SIZE);

    event_read_data_success = 1;
    event_read_data_confirm = 1;

    for ( i = 0; i < num_elements; i++ ) {

        print_debug("Testing callback event_read for element %d\n", i);
        retval = MPI_T_event_read(event, i, element_buffer);

        if (MPI_SUCCESS != retval) {
            print_error("MPI_T_event_read failed", retval, TEST_CONTINUE);
            event_read_data_success = 0;
            error_count++;
        } 

        //  Event index 0 uses match header names
          static char *mca_pml_ob1_match_hdr_names[] = {
              "context id", "source", "tag", "sequence number", NULL,
              };

        if ( array_of_datatypes[i] != 0 ) {

            MPI_Type_get_name( array_of_datatypes[i], type_name, &resultlen);
            MPI_Type_size(array_of_datatypes[i], &type_size);

            if ( type_size == 2 ) {
                print_debug("  [%d] Datatype : %20s  Displacement : %2lu Size : %d : name : %20s val %hd\n", 
                        i, type_name, (unsigned long)array_of_displacements[i], type_size, mca_pml_ob1_match_hdr_names[i], *(short*)element_buffer);
            }
            else if ( type_size == 4 ) {
                print_debug("  [%d] Datatype : %20s  Displacement : %2lu Size : %d : name : %20s val %d\n", 
                        i, type_name, (unsigned long)array_of_displacements[i], type_size, mca_pml_ob1_match_hdr_names[i], *(int*)element_buffer);
            }

            print_debug("element_test_buffer copy is at %d of size %d\n", (array_of_displacements[i]-type_size), type_size);
            memcpy(element_test_buffer+(array_of_displacements[i]), element_buffer, type_size);

            // Check sequence number for increasing integers
            // Specific to OMPI event ID 0, counter index 3 with current MPI activity
            if ( 3 == i )
                print_debug("i is %d, seq_num is %d, sequence counter is %d\n", i, seq_num, *(short*)element_buffer);

            if ( 3 == i && 1 == event_read_data_confirm && seq_num != *(short*)element_buffer ) {
                event_read_data_confirm = 0;
                error_count++;
            }
        }
    }

    seq_num++;


    // For MPI_T_EVENT_COPY, the argument array_of_displacements returns an array of byte displacements in the event buffer in ascending order starting with zero.
    retval = MPI_T_event_copy(event, element_buffer);    
    if (MPI_SUCCESS != retval) {
        print_error("MPI_T_event_failed failed", retval, TEST_CONTINUE);
        if ( 1 == event_copy_data_success ) {
            event_copy_data_success = 0;
            error_count++;
        }
    } 

    for ( i = 0; i < 12; i++ ) {
        print_debug(" %d : %x - %x \n", i, *(char*)(element_test_buffer+i), *(char*)(element_buffer+i));
    }

    //  Length of buffer is last offset + size of last element
    MPI_Type_size(array_of_datatypes[num_elements-1], &type_size);
    element_buffer_length = array_of_displacements[num_elements-1] + type_size;

    if ( 0 == memcmp(element_test_buffer, element_buffer, element_buffer_length) ) {
        print_debug("event_copy buffers match for length %d!\n", element_buffer_length);
    } else {
        print_error("event_copy buffers do not match!\n", NO_MPI_ERROR_CODE, TEST_CONTINUE);
        event_copy_data_match = 0;
    }

    if ( do_failure_tests ) {
        retval = MPI_T_event_read(event, num_elements+1, element_buffer);

        if (MPI_SUCCESS != retval) {
            event_element_index_exceed_handled = 1;
        } 
        else {
            print_error("MPI_T_event_read invalid event element index num_elements+1 not handled", retval, TEST_CONTINUE);
        }

        retval = MPI_T_event_read(event, -1, element_buffer);

        if (MPI_SUCCESS != retval) {
            event_element_index_negative_handled = 1;
        } 
        else {
            print_error("MPI_T_event_read invalid event element index -1 not handled", retval, TEST_CONTINUE);
        } 
    }

    free(array_of_datatypes);
    free(array_of_displacements);
    free(element_buffer);
    free(element_test_buffer);
}


void test_free_event_cb_function(MPI_T_event_registration handle, MPI_T_cb_safety cb_safety, void *user_data) {

    print_debug("In cb_function : %s %s %d\n", __func__, __FILE__, __LINE__);
}


void test_handle_alloc_register_generate_free() {

    int retval;

    int event_index;
    MPI_T_event_registration event_registration;
    char *event_name = "ompi_pml_ob1_message_arrived";

    print_debug("Event Name is %s\n", event_name);
    retval = MPI_T_event_get_index(event_name, &event_index);

    // TEST MPI_T_event_handle_alloc
    //

    print_debug("Testing expected success for index %d\n", event_index);
    //retval = MPI_T_event_handle_alloc(event_index, NULL, MPI_INFO_NULL, &event_registration) ;
    retval = MPI_T_event_handle_alloc(event_index, MPI_COMM_WORLD, MPI_INFO_NULL, &event_registration) ;

    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_handle_alloc did not return MPI_SUCCESS", retval, TEST_EXIT);


    // TEST MPI_T_event_register_callback
    //
    cb_user_data = user_data_string;
    print_debug("Testing expected MPI_T_event_register_callback success for index %d\n", event_index);
    retval = MPI_T_event_register_callback(event_registration, MPI_T_CB_REQUIRE_ASYNC_SIGNAL_SAFE, MPI_INFO_NULL, cb_user_data, test_event_cb_function);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_register_callback did not return MPI_SUCCESS", retval, TEST_EXIT);

    generate_callback_activity();

    // TEST MPI_T_event_handle_free
    //

    retval = MPI_T_event_handle_free(event_registration, cb_user_data, test_free_event_cb_function);
    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_handle_free did not return MPI_SUCCESS", retval, TEST_EXIT);
}


int main (int argc, char** argv)
{

    test_init("MPI_T Events Read Event Data Tests", argc, argv);

    test_handle_alloc_register_generate_free();

    print_results();

    MPI_T_finalize();
    MPI_Finalize();

    return 0;
}

