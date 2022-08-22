#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  MPI_Comm parent, intercomm;

  MPI_Init(NULL, NULL);

  MPI_Comm_get_parent(&parent);
  if (MPI_COMM_NULL != parent)
    MPI_Comm_disconnect(&parent);
  
  if (argc > 1) {
    printf("Spawning '%s' ... ", argv[1]);
    MPI_Comm_spawn(argv[1], MPI_ARGV_NULL,
                   1, MPI_INFO_NULL, 0, MPI_COMM_SELF,
                   &intercomm, MPI_ERRCODES_IGNORE);
    MPI_Comm_disconnect(&intercomm);
    printf("OK\n");
  }

  MPI_Finalize();
}
