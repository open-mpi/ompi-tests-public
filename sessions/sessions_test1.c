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
    MPI_Session session, session1;
    MPI_Errhandler errhandler;
    MPI_Group group;
    MPI_Comm comm_world, comm_self;
    MPI_Info info;
    int rc, npsets, one = 1, sum, i;

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
    if (MPI_SUCCESS != rc) {
        print_error("Session get num psets failed", rc);
        return -1;
    }

    for (i = 0 ; i < npsets ; ++i) {
        int psetlen = 0;
        char name[256];
        MPI_Session_get_nth_pset (session, MPI_INFO_NULL, i, &psetlen, NULL);
        MPI_Session_get_nth_pset (session, MPI_INFO_NULL, i, &psetlen, name);
        fprintf (stderr, "  PSET %d: %s (len: %d)\n", i, name, psetlen);
    }

    rc = MPI_Group_from_session_pset (session, "mpi://WORLD", &group);
    if (MPI_SUCCESS != rc) {
        print_error("Could not get a group for mpi://WORLD", rc);
        return -1;
    }

    rc = MPI_Comm_create_from_group (group, "my_world", MPI_INFO_NULL, MPI_ERRORS_RETURN, &comm_world);
    if (MPI_SUCCESS != rc) {
        print_error("Could not get a comm from group", rc);
        return -1;
    }
    MPI_Group_free (&group);

    MPI_Allreduce (&one, &sum, 1, MPI_INT, MPI_SUM, comm_world);

    fprintf (stderr, "World Comm Sum (1): %d\n", sum);

    rc = MPI_Group_from_session_pset (session, "mpi://SELF", &group);
    if (MPI_SUCCESS != rc) {
        print_error("Could not get a group for mpi://SELF", rc);
        return -1;
    }

    MPI_Comm_create_from_group (group, "myself", MPI_INFO_NULL, MPI_ERRORS_RETURN, &comm_self);
    if (MPI_SUCCESS != rc) {
        print_error("Could not get a comm from group", rc);
        return -1;
    }

    MPI_Group_free (&group);
    MPI_Allreduce (&one, &sum, 1, MPI_INT, MPI_SUM, comm_self);

    fprintf (stderr, "Self Comm Sum (1): %d\n", sum);

    MPI_Errhandler_free (&errhandler);
    MPI_Info_free (&info);


    MPI_Comm_free (&comm_world);
    MPI_Comm_free (&comm_self);
    rc = MPI_Session_finalize (&session);
    if (MPI_SUCCESS != rc) {
        print_error("Session finalize returned error", rc);
        return -1;
    }

    fprintf(stderr, "Finished first finalize\n");

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

    printf("Starting second init\n");
    rc = MPI_Session_init (info, errhandler, &session);
    printf("Finished second init\n");
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
        fprintf (stderr, "  PSET %d: %s (len: %d)\n", i, name, psetlen);
    }

    rc = MPI_Group_from_session_pset (session, "mpi://WORLD", &group);
    if (MPI_SUCCESS != rc) {
        print_error("Could not get a group for mpi://WORLD", rc);
        return -1;
    }

    MPI_Comm_create_from_group (group, "my_world", MPI_INFO_NULL, MPI_ERRORS_RETURN, &comm_world);
    MPI_Group_free (&group);

    MPI_Allreduce (&one, &sum, 1, MPI_INT, MPI_SUM, comm_world);

    fprintf (stderr, "World Comm Sum (1): %d\n", sum);


    rc = MPI_Group_from_session_pset (session, "mpi://SELF", &group);
    if (MPI_SUCCESS != rc) {
        print_error("Could not get a group for mpi://SELF ", rc);
        return -1;
    }

    MPI_Comm_create_from_group (group, "myself", MPI_INFO_NULL, MPI_ERRORS_RETURN, &comm_self);
    MPI_Group_free (&group);
    MPI_Allreduce (&one, &sum, 1, MPI_INT, MPI_SUM, comm_self);

    fprintf (stderr, "Self Comm Sum (1): %d\n", sum);

    MPI_Errhandler_free (&errhandler);
    MPI_Info_free (&info);


    MPI_Comm_free (&comm_world);
    MPI_Comm_free (&comm_self);
    MPI_Session_finalize (&session);

    return 0;
}
