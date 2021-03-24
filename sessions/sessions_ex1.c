#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

static MPI_Session lib_shandle = MPI_SESSION_NULL;
static MPI_Comm lib_comm = MPI_COMM_NULL;

int library_foo_init(void)
{
    int rc, flag;
    int ret = 0;
    const char pset_name[] = "mpi://WORLD";
    const char mt_key[] = "thread_level";
    const char mt_value[] = "MPI_THREAD_MULTIPLE";
    char out_value[100];   /* large enough */
    MPI_Group wgroup = MPI_GROUP_NULL;
    MPI_Info sinfo = MPI_INFO_NULL;
    MPI_Info tinfo = MPI_INFO_NULL;

    MPI_Info_create(&sinfo);
    MPI_Info_set(sinfo, mt_key, mt_value);
    rc = MPI_Session_init(sinfo, MPI_ERRORS_RETURN, 
                          &lib_shandle);
    if (rc != MPI_SUCCESS) {
        ret = -1;
        goto fn_exit;
    }

    /*
     * check we got thread support level foo library needs
     */
    rc = MPI_Session_get_info(lib_shandle, &tinfo);
    if (rc != MPI_SUCCESS) {
        ret = -1;
        goto fn_exit;
    }

    MPI_Info_get(tinfo, mt_key, sizeof(out_value),
                 out_value, &flag);
    if (flag != 1) {
        printf("Could not find key %s\n", mt_key);
        ret = -1;
        goto fn_exit;
    }

    if (strcmp(out_value, mt_value)) {
        printf("Did not get thread multiple support, got %s\n",
               out_value);
        ret = -1;
        goto fn_exit;
    }

    /*
     * create a group from the WORLD process set
     */
    rc = MPI_Group_from_session_pset(lib_shandle,
                                     pset_name,
                                     &wgroup);
    if (rc != MPI_SUCCESS) {
        ret = -1;
        goto fn_exit;
    }

    /*
     * get a communicator
     */
    rc = MPI_Comm_create_from_group(wgroup,
                                    "mpi.forum.mpi-v4_0.example-ex10_8",
                                    MPI_INFO_NULL,
                                    MPI_ERRORS_RETURN,
                                    &lib_comm);
    if (rc != MPI_SUCCESS) {
        ret = -1;
        goto fn_exit;
    }

    /*
     * free group, library doesn't need it.
     */

fn_exit:
    MPI_Group_free(&wgroup);

    if (sinfo != MPI_INFO_NULL) {
	MPI_Info_free(&sinfo);
    }

    if (tinfo != MPI_INFO_NULL) {
	MPI_Info_free(&tinfo);
    }

    if (ret != 0) {
        MPI_Session_finalize(&lib_shandle);
    }

    return ret;
}
