/*
 *  this test disabled for now, has numerous bugs
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

void print_error(const char *msg, int rc)
{
    char err_str[MPI_MAX_ERROR_STRING];
    int resultlen = sizeof(err_str) - 1;

    MPI_Error_string(rc, err_str,  &resultlen);
    fprintf (stderr, "%s return err code  = %d (%s)\n", msg, rc, err_str);
}

void my_session_errhandler (MPI_Session *foo, int *bar, ...)
{
    fprintf(stderr, "errhandler called with error %d\n", *bar);
}

int main (int argc, char *argv[])
{
    MPI_Session session;
    MPI_Errhandler errhandler;
    MPI_Group group;
    MPI_Comm comm_world, parent_comm, inter_comm;
    MPI_Info info;
    int rc, npsets, one = 1, sum, i, np = 4;
    int codes[np];

    MPI_Init(&argc, &argv);

    rc = MPI_Session_create_errhandler (my_session_errhandler, &errhandler);
    if (MPI_SUCCESS != rc) {
        print_error("Error handler creation failed", rc);
        return -1;
    }

    rc = MPI_Info_create (&info);
    if (MPI_SUCCESS != rc) {
        print_error("Info creation failed", rc);
        return -1;
    }

    rc = MPI_Info_set(info, "mpi_thread_support_level", "MPI_THREAD_MULTIPLE");
    if (MPI_SUCCESS != rc) {
        print_error("Info key/val set failed", rc);
        return -1;
    }

    rc = MPI_Session_init (info, errhandler, &session);
    if (MPI_SUCCESS != rc) {
        print_error("Session initialization failed", rc);
        return -1;
    }

    rc = MPI_Session_get_num_psets (session, MPI_INFO_NULL, &npsets);
    for (i = 0 ; i < npsets ; ++i) {
        int psetlen = 0;
        char name[256];
        MPI_Session_get_nth_pset (session, MPI_INFO_NULL, i, &psetlen, NULL);
        MPI_Session_get_nth_pset (session, MPI_INFO_NULL, i, &psetlen, name);
    }

    rc = MPI_Group_from_session_pset (session, "mpi://WORLD", &group);
    if (MPI_SUCCESS != rc) {
        print_error("Could not get a group for mpi://WORLD. ", rc);
        return -1;
    }

    MPI_Comm_create_from_group (group, "my_world", MPI_INFO_NULL, MPI_ERRORS_RETURN, &comm_world);
    MPI_Group_free (&group);

    MPI_Comm_get_parent(&parent_comm);
    if (parent_comm == MPI_COMM_NULL) {
        MPI_Comm_spawn("./sessions_test14", MPI_ARGV_NULL, np, MPI_INFO_NULL, 0, comm_world, &inter_comm, codes);
    } else {
        MPI_Comm   myComm;       /* intra-communicator of local sub-group */ 
        MPI_Comm   myFirstComm;  /* inter-communicator */ 
        MPI_Comm   mySecondComm; /* second inter-communicator (group 1 only) */ 
        int membershipKey; 
        int rank;
        MPI_Comm_rank(parent_comm, &rank);

        /* User code must generate membershipKey in the range [0, 1, 2] */ 
        membershipKey = rank % 3; 

        /* Build intra-communicator for local sub-group */ 
        MPI_Comm_split(comm_world, membershipKey, rank, &myComm); 

        /* Build inter-communicators.  Tags are hard-coded. */ 
        if (membershipKey == 0) 
        {                     /* Group 0 communicates with group 1. */ 
        MPI_Intercomm_create( myComm, 0, parent_comm, 1, 
                            1, &myFirstComm); 
        } 
        else if (membershipKey == 1) 
        {              /* Group 1 communicates with groups 0 and 2. */ 
        MPI_Intercomm_create( myComm, 0, parent_comm, 0, 
                            1, &myFirstComm); 
        MPI_Intercomm_create( myComm, 0, parent_comm, 2, 
                            12, &mySecondComm); 
        } 
        else if (membershipKey == 2) 
        {                     /* Group 2 communicates with group 1. */ 
        MPI_Intercomm_create( myComm, 0, parent_comm, 1, 
                            12, &myFirstComm); 
        }

        switch(membershipKey)  /* free communicators appropriately */ 
        { 
        case 1: 
        MPI_Comm_free(&mySecondComm); 
        case 0: 
        case 2: 
        MPI_Comm_free(&myFirstComm); 
        break; 
        }
    }

    MPI_Errhandler_free (&errhandler);
    MPI_Info_free (&info);

    MPI_Comm_free (&comm_world);
    MPI_Session_finalize (&session);
    printf("Done\n");

    MPI_Finalize();

    return 0;
}
