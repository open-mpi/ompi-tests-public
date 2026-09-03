/*
 * events_types.c
 *
 * Test events type functions for event count and info
 *  
 */

#include "events_common.h"


int event_count = 0;


//  Test flags
int event_info_success = 0;
int event_info_failed = 0;
int event_info_displacements_start_at_0 = 1;
int event_count_result = 0;
int event_get_num_handle_null = 0;
int event_info_negative_name_len_handled = 1;
int event_info_negative_desc_len_handled = 1;


void print_results() {

    if ( 0 != rank ) return;

    fprintf(outstream, "%-*s - %-*s : %6d\n", func_width, "MPI_T_event_get_num", metric_width, "Event count", event_count);
    print_pf_result("MPI_T_event_get_num", "Successful return", event_count_result);
    print_pf_result("MPI_T_event_get_info", "Successful return", event_info_success);
    print_pf_result("MPI_T_event_get_info", "Displacements start at 0", event_info_displacements_start_at_0);
    fprintf(outstream, "%-*s - %-*s : %6d\n", func_width, "MPI_T_event_get_info", metric_width, "Successful event calls", event_info_success);
    fprintf(outstream, "%-*s - %-*s : %6d\n", func_width, "MPI_T_event_get_info", metric_width, "Unexpected Failed event calls", event_info_failed);

    if ( do_failure_tests ) {
        print_pf_result("MPI_T_event_get_num", "Handle NULL argument", event_get_num_handle_null);
        print_pf_result("MPI_T_event_get_info", "Handle negative name length", event_info_negative_name_len_handled);
        print_pf_result("MPI_T_event_get_info", "Handle negative desc length", event_info_negative_desc_len_handled);
    }

    fprintf(outstream, "%-*s - %-*s : %6d\n", func_width, "TOTAL ERROR COUNT", metric_width, "", error_count);
}


void test_get_num() {

    int retval;

    retval = MPI_T_event_get_num(&event_count);

    if ( retval != MPI_SUCCESS )
        print_error("MPI_T_event_get_num did not return MPI_SUCCESS", retval, TEST_EXIT);
    else
        event_count_result = 1;


    if ( do_failure_tests ) {

        retval = MPI_T_event_get_num(NULL);

        if ( retval != MPI_ERR_ARG ) {
            fprintf(errout, "Expected value %d, got %d\n", MPI_ERR_ARG, retval);
            print_error("MPI_T_event_get_num did not return MPI_ERR_ARG", retval, TEST_CONTINUE);
        }
        else
            event_get_num_handle_null = 1;
    }
}


void test_get_info() {
    int i, d;
    char type_name[MPI_MAX_OBJECT_NAME];
    int retval, resultlen;

    EVENT_INFO *infos;
    EVENT_INFO ci;

    infos = (EVENT_INFO*)malloc (event_count * sizeof(EVENT_INFO));
    if ( NULL == infos )
        print_error("Failed to allocate event info object!!!", -1, TEST_EXIT);
 
    /*
     * get_info requirements
     *
     * subsequent calls to this routine that query information about the same event type must return the same information 
     * returns MPI_T_ERR_INVALID_INDEX if event_index does not match a valid event type index
     */

    for ( i = 0; i < event_count; i++ )
    {

        ci.event_index = i;

        // int MPI_T_event_get_info(int event_index, char *name, int *name_len,
        //   int *verbosity, MPI_Datatype *array_of_datatypes,
        //   MPI_Aint *array_of_displacements, int *num_elements,
        //   MPI_T_enum *enumtype, MPI_Info* info,
        //   char *desc, int *desc_len, int *bind)    
        //
        /*  Get lengths */
        retval = MPI_T_event_get_info (
                ci.event_index, 
                NULL,
                &(ci.name_len),
                &(ci.verbosity),
                NULL,
                NULL,
                &(ci.num_elements),
                &(ci.enumtype),
                NULL,
                NULL,
                &(ci.desc_len),
                &(ci.bind)
                );

        if (MPI_SUCCESS != retval && MPI_T_ERR_INVALID_INDEX != retval ) {
            if ( MPI_T_ERR_INVALID_INDEX != retval ) {
                fprintf(errout, "Expected value %d, got %d\n", MPI_T_ERR_INVALID_INDEX, retval);
            }
            print_error("MPI_T_event_get_info Invalid return value", -1, TEST_CONTINUE);
        } 
        
        if (MPI_SUCCESS != retval ) {
            print_error("Failed to get event info", retval, TEST_CONTINUE);
            event_info_failed++;
            ci.name_len = 0;
            memcpy(&infos[i], &ci, sizeof(EVENT_INFO));
            continue;
        }
        else {
            event_info_success++;
        }
        
        //  allocate strings and arrays for datatypes and displacements
        ci.name = (char*)malloc(ci.name_len);
        ci.array_of_datatypes = (MPI_Datatype*)malloc(ci.num_elements*sizeof(MPI_Datatype));
        ci.array_of_displacements = (MPI_Aint*)malloc(ci.num_elements*sizeof(MPI_Aint));
        ci.desc = (char*)malloc(ci.desc_len);

        if ( !ci.name || ! ci.array_of_datatypes || !ci.array_of_displacements || !ci.desc )
            print_error("Failed to allocate info name and description buffers", retval, TEST_EXIT);

        /*  Get data */
        retval = MPI_T_event_get_info(
                ci.event_index, 
                ci.name,
                &(ci.name_len),
                &(ci.verbosity),
                ci.array_of_datatypes,
                ci.array_of_displacements,
                &(ci.num_elements),
                &(ci.enumtype),
                &(ci.info),
                ci.desc,
                &(ci.desc_len),
                &(ci.bind)
                ) ;
        if (MPI_SUCCESS != retval ) {
            print_error("Failed to get event info", retval, TEST_CONTINUE);
            event_info_failed++;
        }
        else {
            event_info_success++;
        }

        memcpy(&infos[i], &ci, sizeof(EVENT_INFO));

        if ( 1 == event_info_displacements_start_at_0 && 0 != ci.array_of_displacements[0] ) {
            print_error("Event_info displacements do not start at 0", retval, TEST_CONTINUE);
            event_info_displacements_start_at_0 = 0;
        }
    } 

    //  Print event info and elements
    if ( print_events && !rank ) {
        for ( i = 0; i < event_count; i++ ) {

            if ( infos[i].name_len < 1 ) {
                print_debug("Event[%7d] : Unavailable\n", infos[i].event_index);
                continue;
            }

            fprintf(outstream, "Event[%7d] : %-40s : %-50.*s : %5d : %s\n", 
                    infos[i].event_index, 
                    infos[i].name,
                    MAX_STRING-1,
                    infos[i].desc,
                    infos[i].num_elements,
                    bind_str(infos[i].bind)
                  );

            for ( d = 0; d < infos[i].num_elements; d++ ) {
    
                if ( infos[i].array_of_datatypes[d] != 0 ) {
                    MPI_Type_get_name( infos[i].array_of_datatypes[d], type_name, &resultlen);
                    fprintf(outstream, "  [%d] Datatype : %20s  Displacement : %lu\n", d, type_name, (unsigned long)infos[i].array_of_displacements[d]);
                }
            }

            fprintf(outstream, "\n");

        }
    }


    if ( do_failure_tests ) {

        int save_name_len;

        save_name_len = ci.name_len;

        /* name_len and desc_len are INOUT arguments and should handle negative values
         * Ideally it would return MPI_ERR_ARG (although not specified in MPI Standard v4.0),
         * but otherwise it should at least not segfault.*/
        ci.name_len = -100;
        print_debug("Testing MPI_T_event_get_info with negative name_len\n");
        retval = MPI_T_event_get_info(
                ci.event_index,
                ci.name,
                &(ci.name_len),
                &(ci.verbosity),
                ci.array_of_datatypes,
                ci.array_of_displacements,
                &(ci.num_elements),
                &(ci.enumtype),
                &(ci.info),
                ci.desc,
                &(ci.desc_len),
                &(ci.bind)
                ) ;

        if (MPI_ERR_ARG != retval && 1 == event_info_negative_name_len_handled ) {
            event_info_negative_name_len_handled = 0;
        }

        ci.name_len = save_name_len;
        ci.desc_len = -100;

        print_debug("Testing MPI_T_event_get_info with negative name_len\n");
        retval = MPI_T_event_get_info(
                ci.event_index,
                ci.name,
                &(ci.name_len),
                &(ci.verbosity),
                ci.array_of_datatypes,
                ci.array_of_displacements,
                &(ci.num_elements),
                &(ci.enumtype),
                &(ci.info),
                ci.desc,
                &(ci.desc_len),
                &(ci.bind)
                ) ;

        if (MPI_ERR_ARG != retval && 1 == event_info_negative_desc_len_handled ) {
            event_info_negative_desc_len_handled = 0;
        }
    }

    free(ci.name);
    free(ci.desc);
    free(infos);
}


int main (int argc, char** argv)
{
    test_init("MPI_T Events Type Tests", argc, argv);

    test_get_num();
    test_get_info();

    print_results();

    MPI_T_finalize();
    MPI_Finalize();

    return 0;
}

