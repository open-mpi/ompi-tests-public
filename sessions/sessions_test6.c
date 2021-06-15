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
    MPI_Session session, session1;
    MPI_Errhandler errhandler;
    MPI_Group group, group1;
    MPI_Comm comm_world, comm_self, comm_world1, comm_self1;
    MPI_Info info, info1;
    int rc, npsets, npsets1, one = 1, sum, sum1, i;

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

    MPI_Allreduce (&one, &sum, 1, MPI_INT, MPI_SUM, comm_world);

    fprintf (stderr, "World Comm Sum (1): %d\n", sum);

    rc = MPI_Group_from_session_pset (session, "mpi://SELF", &group);
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Could not get a group for mpi://SELF. rc = %d\n", rc);
        abort ();
    }

    MPI_Comm_create_from_group (group, "myself", MPI_INFO_NULL, MPI_ERRORS_RETURN, &comm_self);
    MPI_Group_free (&group);
    MPI_Allreduce (&one, &sum, 1, MPI_INT, MPI_SUM, comm_self);

    fprintf (stderr, "Self Comm Sum (1): %d\n", sum);

    MPI_Comm_free (&comm_world);
    MPI_Comm_free (&comm_self);
    MPI_Errhandler_free (&errhandler);
    MPI_Info_free (&info);
    MPI_Session_finalize (&session);
    printf("Finished first finalize\n");

    rc = MPI_Session_create_errhandler (my_session_errhandler, &errhandler);
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Error handler creation failed with rc = %d\n", rc);
        abort ();
    }
    
    rc = MPI_Info_create (&info1);
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Info creation failed with rc = %d\n", rc);
        abort ();
    }

    rc = MPI_Info_set(info1, "mpi_thread_support_level", "MPI_THREAD_MULTIPLE");
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Info key/val set failed with rc = %d\n", rc);
        abort ();
    }

    printf("Starting second init\n");
    rc = MPI_Session_init (info1, errhandler, &session1);
    printf("Finished second init\n");
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Session1 initialization failed with rc = %d\n", rc);
        abort ();
    }

    rc = MPI_Session_get_num_psets (session1, MPI_INFO_NULL, &npsets1);
    for (i = 0 ; i < npsets1 ; ++i) {
        int psetlen = 0;
        char name[256];
        MPI_Session_get_nth_pset (session1, MPI_INFO_NULL, i, &psetlen, NULL);
        MPI_Session_get_nth_pset (session1, MPI_INFO_NULL, i, &psetlen, name);
        fprintf (stderr, "  PSET1 %d: %s (len: %d)\n", i, name, psetlen);
    }

    rc = MPI_Group_from_session_pset (session1, "mpi://WORLD", &group1);
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Could not get a group1 for mpi://WORLD. rc = %d\n", rc);
        abort ();
    }

    MPI_Comm_create_from_group (group1, "my_world1", MPI_INFO_NULL, MPI_ERRORS_RETURN, &comm_world1);
    MPI_Group_free (&group1);

    MPI_Allreduce (&one, &sum1, 1, MPI_INT, MPI_SUM, comm_world1);

    fprintf (stderr, "World Comm Sum1 (1): %d\n", sum1);

    rc = MPI_Group_from_session_pset (session1, "mpi://SELF", &group1);
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Could not get a group1 for mpi://SELF. rc = %d\n", rc);
        abort ();
    }

    MPI_Comm_create_from_group (group1, "myself1", MPI_INFO_NULL, MPI_ERRORS_RETURN, &comm_self1);
    MPI_Group_free (&group1);

    MPI_Allreduce (&one, &sum1, 1, MPI_INT, MPI_SUM, comm_self1);

    fprintf (stderr, "Self Comm Sum1 (1): %d\n", sum1);

    MPI_Info_free (&info1);
    MPI_Comm_free (&comm_world1);
    MPI_Comm_free (&comm_self1);
    MPI_Session_finalize (&session1);

    return 0;
}
