/* 
 * P. 121, line 11: Calling MPI_PARRIVED on a request that does not correspond to a partitioned receive operation 
 * is erroneous.
 *
 * This test uses PARRIVED on a request corresponding to an Irecv
 *
 * Expected outcome: some sort of error
 *
 */

#include "test_common.h" 

#define COUNT 32

int main(int argc, char *argv[]) {

    int message[COUNT];

    int source = 0, dest = 1, tag = 1, flag = 0; 
    int myrank, i, j;
    int provided;

    for (i = 0; i < COUNT; ++i) message[i] = 0;

    MPI_Request request;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided); 
    if (provided < MPI_THREAD_SERIALIZED) MPI_Abort(MPI_COMM_WORLD , EXIT_FAILURE); 
    MPI_Comm_rank(MPI_COMM_WORLD , &myrank);

    if (0 == myrank) {

        for (i = 0; i < COUNT; ++i) message[i] = i;

        MPI_Isend(message, COUNT, MPI_INT, dest, tag, MPI_COMM_WORLD, &request);

        while (!flag) {
            CHECK_RETVAL(MPI_Test(&request, &flag, MPI_STATUS_IGNORE)); 
        }

        //MPI_Request_free(&request);

    }
    else if (1 == myrank) {

        MPI_Irecv(message, COUNT, MPI_INT, source, tag, MPI_COMM_WORLD, &request);

        /* this is erroneous */
        MPI_Parrived(request, 0, &flag);

        /* continue as usual */
        MPI_Wait(&request, MPI_STATUS_IGNORE);

        for (i=0; i < COUNT; ++i) {
            fprintf(stderr,"%d ", message[i]);
        }
        fprintf(stderr, "\n");

        /* all partitions received; check contents */
        for (i = 0; i < COUNT; ++i) {
            if (message[i] != i) {
                fprintf(stderr, "ERROR: Contents received do not match contents sent (expected %d, found %d).\n",i,message[i]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == myrank) {TEST_RAN_TO_COMPLETION();}

    MPI_Finalize();

    return 0;
}

