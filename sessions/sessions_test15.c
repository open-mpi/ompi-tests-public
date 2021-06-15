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
    MPI_Comm comm_world, comm_world1;
    MPI_Info info;
    int rc, npsets, npsets1, one = 1, i;

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

    rc = MPI_Session_init (info, errhandler, &session1);
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Session1 initialization failed with rc = %d\n", rc);
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

    rc = MPI_Session_get_num_psets (session1, MPI_INFO_NULL, &npsets1);
    for (i = 0 ; i < npsets1 ; ++i) {
        int psetlen = 0;
        char name[256];
        MPI_Session_get_nth_pset (session1, MPI_INFO_NULL, i, &psetlen, NULL);
        MPI_Session_get_nth_pset (session1, MPI_INFO_NULL, i, &psetlen, name);
        fprintf (stderr, "  PSET1 %d: %s (len: %d)\n", i, name, psetlen);
    }

    rc = MPI_Group_from_session_pset (session, "mpi://WORLD", &group);
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Could not get a group for mpi://WORLD. rc = %d\n", rc);
        abort ();
    }
    
    rc = MPI_Group_from_session_pset (session1, "mpi://WORLD", &group1);
    if (MPI_SUCCESS != rc) {
        fprintf (stderr, "Could not get a group1 for mpi://WORLD. rc = %d\n", rc);
        abort ();
    }

    MPI_Comm_create_from_group (group, "my_world", MPI_INFO_NULL, MPI_ERRORS_RETURN, &comm_world);
    MPI_Group_free (&group);

    MPI_Comm_create_from_group (group1, "my_world", MPI_INFO_NULL, MPI_ERRORS_RETURN, &comm_world1);
    MPI_Group_free (&group1);

    int rank, buffer;
    MPI_Comm_rank(comm_world, &rank);

    /* Check MPI_Waitall */
    if (rank == 0) {
        buffer = 1;
        MPI_Request requests[3];
        MPI_Isend(&buffer, 1, MPI_INT, 1, 0, comm_world, &requests[0]);
        MPI_Isend(&buffer, 1, MPI_INT, 2, 0, comm_world1, &requests[1]);
        MPI_Isend(&buffer, 1, MPI_INT, 3, 0, comm_world, &requests[2]);

        MPI_Waitall(3, requests, MPI_STATUSES_IGNORE);
    } else if (rank == 2) {
        buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, 0, 0, comm_world1, MPI_STATUS_IGNORE);
    } else {
        buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, 0, 0, comm_world, MPI_STATUS_IGNORE);
    }

    /* Check MPI_Waitany */
    if (rank == 0) {
        buffer = 1;
        MPI_Request requests[3];
        int request_ranks[3];
        MPI_Isend(&buffer, 1, MPI_INT, 1, 0, comm_world, &requests[0]);
        request_ranks[0] = 1;
        MPI_Isend(&buffer, 1, MPI_INT, 2, 0, comm_world1, &requests[1]);
        request_ranks[0] = 2;
        MPI_Isend(&buffer, 1, MPI_INT, 3, 0, comm_world, &requests[2]);
        request_ranks[0] = 3;

        int index;
        MPI_Waitany(3, requests, &index, MPI_STATUSES_IGNORE);
    } else if (rank == 2) {
        buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, 0, 0, comm_world1, MPI_STATUS_IGNORE);
    } else {
        buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, 0, 0, comm_world, MPI_STATUS_IGNORE);
    }

    /* Check MPI_Waitsome */
    if (rank == 0) {
        buffer = 1;
        MPI_Request requests[3];
        int request_ranks[3];
        MPI_Isend(&buffer, 1, MPI_INT, 1, 0, comm_world, &requests[0]);
        request_ranks[0] = 1;
        MPI_Isend(&buffer, 1, MPI_INT, 2, 0, comm_world1, &requests[1]);
        request_ranks[0] = 2;
        MPI_Isend(&buffer, 1, MPI_INT, 3, 0, comm_world, &requests[2]);
        request_ranks[0] = 3;

        int index_count;
        int indices[3];
        MPI_Waitsome(3, requests, &index_count, indices, MPI_STATUSES_IGNORE);
    } else if (rank == 2) {
        buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, 0, 0, comm_world1, MPI_STATUS_IGNORE);
    } else {
        buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, 0, 0, comm_world, MPI_STATUS_IGNORE);
    }

    /* Check MPI_Testall */
    if (rank == 0) {
        buffer = 1;
        MPI_Request requests[3];
        MPI_Isend(&buffer, 1, MPI_INT, 1, 0, comm_world, &requests[0]);
        MPI_Isend(&buffer, 1, MPI_INT, 2, 0, comm_world1, &requests[1]);
        MPI_Isend(&buffer, 1, MPI_INT, 3, 0, comm_world, &requests[2]);

        int flag;
        MPI_Testall(3, requests, &flag, MPI_STATUSES_IGNORE);
    } else if (rank == 2) {
        buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, 0, 0, comm_world1, MPI_STATUS_IGNORE);
    } else {
        buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, 0, 0, comm_world, MPI_STATUS_IGNORE);
    }

    /* Check MPI_Testany */
    if (rank == 0) {
        buffer = 1;
        MPI_Request requests[3];
        int request_ranks[3];
        MPI_Isend(&buffer, 1, MPI_INT, 1, 0, comm_world, &requests[0]);
        request_ranks[0] = 1;
        MPI_Isend(&buffer, 1, MPI_INT, 2, 0, comm_world1, &requests[1]);
        request_ranks[0] = 2;
        MPI_Isend(&buffer, 1, MPI_INT, 3, 0, comm_world, &requests[2]);
        request_ranks[0] = 3;

        int index, flag;
        MPI_Testany(3, requests, &index, &flag, MPI_STATUSES_IGNORE);
    } else if (rank == 2) {
        buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, 0, 0, comm_world1, MPI_STATUS_IGNORE);
    } else {
        buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, 0, 0, comm_world, MPI_STATUS_IGNORE);
    }

    /* Check MPI_Testsome */
    if (rank == 0) {
        buffer = 1;
        MPI_Request requests[3];
        int request_ranks[3];
        MPI_Isend(&buffer, 1, MPI_INT, 1, 0, comm_world, &requests[0]);
        request_ranks[0] = 1;
        MPI_Isend(&buffer, 1, MPI_INT, 2, 0, comm_world1, &requests[1]);
        request_ranks[0] = 2;
        MPI_Isend(&buffer, 1, MPI_INT, 3, 0, comm_world, &requests[2]);
        request_ranks[0] = 3;

        int index_count;
        int indices[3];
        MPI_Testsome(3, requests, &index_count, indices, MPI_STATUSES_IGNORE);
    } else if (rank == 2) {
        buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, 0, 0, comm_world1, MPI_STATUS_IGNORE);
    } else {
        buffer = 0;
        MPI_Recv(&buffer, 1, MPI_INT, 0, 0, comm_world, MPI_STATUS_IGNORE);
    }

    MPI_Errhandler_free (&errhandler);
    MPI_Info_free (&info);


    MPI_Comm_free (&comm_world);
    MPI_Comm_free (&comm_world1);
    MPI_Session_finalize (&session);
    MPI_Session_finalize (&session1);

    return 0;
}
