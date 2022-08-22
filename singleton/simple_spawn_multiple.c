#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
  MPI_Comm parent, intercomm;
  int maxprocs[2];
  char *command[2];
  char **spawn_argv[2];
  char *argv0[2];
  char *argv1[2];
  MPI_Info info[2];

  MPI_Init(NULL, NULL);

  MPI_Comm_get_parent(&parent);
  if (MPI_COMM_NULL != parent) {
      printf("Hello from a Child (%s)\n", argv[1]);
      fflush(NULL);
      MPI_Barrier(parent);
      MPI_Comm_disconnect(&parent);
  }
  else if(argc > 1) {
      maxprocs[0] = 1;
      maxprocs[1] = 2;
      command[0] = strdup(argv[1]);
      command[1] = strdup(argv[1]);
      spawn_argv[0] = argv0;
      spawn_argv[1] = argv1;
      argv0[0] = strdup("A");
      argv0[1] = NULL;
      argv1[0] = strdup("B");
      argv1[1] = NULL;
      info[0] = MPI_INFO_NULL;
      info[1] = MPI_INFO_NULL;

      printf("Spawning Multiple '%s' ... ", argv[1]);
      MPI_Comm_spawn_multiple(2, command, spawn_argv, maxprocs, info,
                              0, MPI_COMM_SELF,
                              &intercomm, MPI_ERRCODES_IGNORE);
      MPI_Barrier(intercomm);
      MPI_Comm_disconnect(&intercomm);
      printf("OK\n");
  }

  MPI_Finalize();
}
