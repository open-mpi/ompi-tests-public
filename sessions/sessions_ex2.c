#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
    int i, n_psets, psetlen, rc, ret;
    int valuelen;
    int flag = 0;
    char *pset_name = NULL;
    char *info_val = NULL;
    MPI_Session shandle = MPI_SESSION_NULL;
    MPI_Info sinfo = MPI_INFO_NULL;
    MPI_Group pgroup = MPI_GROUP_NULL;

    if (argc < 2) {
        fprintf(stderr, "A process set name fragment is required\n");
        return -1;
    }

    rc = MPI_Session_init(MPI_INFO_NULL, MPI_ERRORS_RETURN, &shandle);
    if (rc != MPI_SUCCESS) {
        fprintf(stderr, "Could not initialize session, bailing out\n");
        return -1;
    }

    MPI_Session_get_num_psets(shandle, MPI_INFO_NULL, &n_psets);

    for (i=0, pset_name=NULL; i<n_psets; i++) {
         psetlen = 0;
         MPI_Session_get_nth_pset(shandle, MPI_INFO_NULL, i,
                                  &psetlen, NULL);
         pset_name = (char *)malloc(sizeof(char) * psetlen);
         MPI_Session_get_nth_pset(shandle, MPI_INFO_NULL, i,
                                  &psetlen, pset_name);
         if (strstr(pset_name, argv[1]) != NULL) break;

         free(pset_name);
         pset_name = NULL;
    }

    /*
     * get instance of an info object for this Session
     */

    MPI_Session_get_pset_info(shandle, pset_name, &sinfo);
    MPI_Info_get_valuelen(sinfo, "size", &valuelen, &flag);
    info_val = (char *)malloc(valuelen+1);
    MPI_Info_get(sinfo, "size", valuelen, info_val, &flag);
    free(info_val);

    /*
     * create a group from the process set
     */

    rc = MPI_Group_from_session_pset(shandle, pset_name,
                                     &pgroup);
    ret = (rc == MPI_SUCCESS) ? 0 : -1;

    free(pset_name);
    MPI_Group_free(&pgroup);
    MPI_Info_free(&sinfo);
    MPI_Session_finalize(&shandle);

    fprintf(stderr, "Test completed ret = %d\n", ret);
    return ret;

}
