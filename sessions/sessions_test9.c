#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

void my_session_errhandler (MPI_Session *foo, int *bar, ...)
{
    fprintf(stderr, "errhandler called with error %d\n", *bar);
}

int main (int argc, char *argv[])
{
    MPI_Session session;
    MPI_Errhandler errhandler;
    MPI_Group group;
    MPI_Comm comm_world;
    MPI_Info info;
    int rc, npsets, one = 1, sum, i;

    MPI_Init(&argc, &argv);

    rc = MPI_Session_create_errhandler (my_session_errhandler, &errhandler);
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Error handler creation failed with rc = %d\n", rc);
        abort ();
    }

    rc = MPI_Info_create (&info);
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Info creation failed with rc = %d\n", rc);
        abort ();
    }

    rc = MPI_Info_set(info, "mpi_thread_support_level", "MPI_THREAD_MULTIPLE");
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Info key/val set failed with rc = %d\n", rc);
        abort ();
    }

    rc = MPI_Session_init (info, errhandler, &session);
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Session initialization failed with rc = %d\n", rc);
        abort ();
    }

    rc = MPI_Session_get_num_psets (session, MPI_INFO_NULL, &npsets);
    for (i = 0 ; i < npsets ; ++i) {
        int psetlen = 0;
        char name[256];
        MPI_Session_get_nth_pset (session, MPI_INFO_NULL, i, &psetlen, NULL);
        MPI_Session_get_nth_pset (session, MPI_INFO_NULL, i, &psetlen, name);
        fprintf (stderr, "  PSET %d: %s (len: %d)\n", i, name, psetlen);
    }

    rc = MPI_Group_from_session_pset (session, "mpi://WORLD", &group);
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Could not get a group for mpi://WORLD. rc = %d\n", rc);
        abort ();
    }

    MPI_Comm_create_from_group (group, "my_world", MPI_INFO_NULL, MPI_ERRORS_RETURN, &comm_world);
    MPI_Group_free (&group);

    MPI_Comm   myComm;       /* intra-communicator of local sub-group */ 
    MPI_Comm   myFirstComm;  /* inter-communicator */ 
    MPI_Comm   mySecondComm; /* second inter-communicator (group 1 only) */ 
    int membershipKey; 
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* User code must generate membershipKey in the range [0, 1, 2] */ 
    membershipKey = rank % 3; 

    /* Build intra-communicator for local sub-group */ 
    MPI_Comm_split(comm_world, membershipKey, rank, &myComm); 

    /* Build inter-communicators.  Tags are hard-coded. */ 
    if (membershipKey == 0) 
    {                     /* Group 0 communicates with group 1. */ 
    MPI_Intercomm_create( myComm, 0, MPI_COMM_WORLD, 1, 
                        1, &myFirstComm); 
    } 
    else if (membershipKey == 1) 
    {              /* Group 1 communicates with groups 0 and 2. */ 
    MPI_Intercomm_create( myComm, 0, MPI_COMM_WORLD, 0, 
                        1, &myFirstComm); 
    MPI_Intercomm_create( myComm, 0, MPI_COMM_WORLD, 2, 
                        12, &mySecondComm); 
    } 
    else if (membershipKey == 2) 
    {                     /* Group 2 communicates with group 1. */ 
    MPI_Intercomm_create( myComm, 0, MPI_COMM_WORLD, 1, 
                        12, &myFirstComm); 
    } 

    /* Do work ... */ 

    switch(membershipKey)  /* free communicators appropriately */ 
    { 
    case 1: 
    MPI_Comm_free(&mySecondComm); 
    case 0: 
    case 2: 
    MPI_Comm_free(&myFirstComm); 
    break; 
    }

    MPI_Errhandler_free (&errhandler);
    MPI_Info_free (&info);

    MPI_Comm_free (&comm_world);
    MPI_Session_finalize (&session);
    printf("Done\n");

    MPI_Finalize();

    return 0;
}
