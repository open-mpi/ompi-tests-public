/*
 * events_example.c
 *
 * Events example that registers callback, generates callback activity, and reports event data
 *  
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>


char *cb_user_data;
int rank, wsize;
int event_index = 0;
char user_data_string[256] = "Test String";
MPI_T_event_registration event_registration;


void event_cb_function(MPI_T_event_instance event, MPI_T_event_registration handle, MPI_T_cb_safety cb_safety, void *user_data) {

    MPI_Count event_timestamp;
    int sequence_index = 3;
    short seq;

    MPI_T_event_get_timestamp(event, &event_timestamp);
    MPI_T_event_read(event, sequence_index, &seq);

    if ( !rank ) fprintf(stdout, "In %s %s %d ts=%lu idx=%hd\n", __func__, __FILE__, __LINE__, (unsigned long)event_timestamp, seq);
}


void free_event_cb_function(MPI_T_event_registration handle, MPI_T_cb_safety cb_safety, void *user_data) {

    if ( !rank ) fprintf(stdout, "In %s %s %d\n", __func__, __FILE__, __LINE__);
}


void register_callback() {

    int name_len, desc_len, num_elements;
    char *name, *desc;

    // int MPI_T_event_get_info(int event_index, char *name, int *name_len,
    //   int *verbosity, MPI_Datatype *array_of_datatypes,
    //   MPI_Aint *array_of_displacements, int *num_elements,
    //   MPI_T_enum *enumtype, MPI_Info* info,
    //   char *desc, int *desc_len, int *bind)    
    //

    MPI_T_event_get_info (
            event_index, 
            NULL,
            &name_len,
            NULL,
            NULL,
            NULL,
            &num_elements,
            NULL,
            NULL,
            NULL,
            &desc_len,
            NULL
            );

    name = (char*) malloc(name_len+1);
    desc = (char*) malloc(desc_len+1);

    MPI_T_event_get_info (
            event_index, 
            name,
            &name_len,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            desc,
            &desc_len,
            NULL
            );

    if ( !rank ) fprintf(stdout, "Registering event index %d : %s : %s\n", event_index, name, desc);
    MPI_T_event_handle_alloc(event_index, MPI_COMM_WORLD, MPI_INFO_NULL, &event_registration) ;

    cb_user_data = (void*)user_data_string;

    MPI_T_event_register_callback(event_registration, MPI_T_CB_REQUIRE_ASYNC_SIGNAL_SAFE, MPI_INFO_NULL, cb_user_data, event_cb_function);

    free(name);
    free(desc);
}


void free_event() {

    MPI_T_event_handle_free(event_registration, cb_user_data, free_event_cb_function);
}


void generate_callback_activity() {

#define GENERATE_BUFFER_SIZE 32

    char buf[GENERATE_BUFFER_SIZE];
    int i, lim = 5;
    MPI_Status stat;
    MPI_Request req;

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


int main (int argc, char** argv)
{

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &wsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int required = MPI_THREAD_SINGLE;
    int provided = 0;

    if ( argc > 1 )
        event_index = atoi(argv[1]);
    else
        event_index = 0;

    MPI_T_init_thread(required, &provided);

    register_callback();

    if ( !rank ) fprintf(stdout,"Registered callback, Generate callback activity.\n");
    generate_callback_activity();

    free_event();

    if ( !rank ) fprintf(stdout,"Freed callback, should not generate callback activity.\n");
    generate_callback_activity();
    
    MPI_T_finalize();
    MPI_Finalize();

    return 0;
}

