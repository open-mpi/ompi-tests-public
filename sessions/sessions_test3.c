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
    MPI_Info info;
    int rc, npsets, i;

    rc = MPI_Session_create_errhandler (my_session_errhandler, &errhandler);
    if (MPI_SUCCESS != rc) {
        print_error ("Error handler creation failed with rc = %d\n", rc);
        return -1;
    }

    rc = MPI_Info_create (&info);
    if (MPI_SUCCESS != rc) {
        print_error ("Info creation failed with rc = %d\n", rc);
        return -1;
    }

    rc = MPI_Info_set(info, "mpi_thread_support_level", "MPI_THREAD_MULTIPLE");
    if (MPI_SUCCESS != rc) {
        print_error ("Info key/val set failed with rc = %d\n", rc);
        return -1;
    }

    rc = MPI_Session_init (info, errhandler, &session);
    if (MPI_SUCCESS != rc) {
        print_error ("Session initialization failed with rc = %d\n", rc);
        return -1;
    }

    rc = MPI_Session_get_num_psets (session, MPI_INFO_NULL, &npsets);
    for (i = 0 ; i < npsets ; ++i) {
        int psetlen = 0;
        char name[256];
        MPI_Session_get_nth_pset (session, MPI_INFO_NULL, i, &psetlen, NULL);
        MPI_Session_get_nth_pset (session, MPI_INFO_NULL, i, &psetlen, name);
    }

    rc = MPI_Group_from_session_pset (session, "mpi://INVALID", &group);
    if (MPI_SUCCESS != rc) {
        print_error ("Could not get a group for mpi://INVALID. rc = %d\n", rc);
        printf("sessions_test3 passed\n");
        MPI_Errhandler_free (&errhandler);
        MPI_Info_free (&info);
        MPI_Session_finalize (&session);
        return 0;
    } else {
        printf("Use of invalid pset failed to throw an error!\n");
        printf("sessions_test3 failed\n");
        MPI_Errhandler_free (&errhandler);
        MPI_Info_free (&info);
        MPI_Session_finalize (&session);
        return -1;
    }

    

}