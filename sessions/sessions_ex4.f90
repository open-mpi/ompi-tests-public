program main
    use mpi
    implicit none
    integer :: pset_len, ierror, n_psets
    character(len=:), allocatable :: pset_name
    integer :: shandle
    integer :: pgroup
    integer :: pcomm

    call MPI_Session_init(MPI_INFO_NULL, MPI_ERRORS_RETURN, &
                          shandle, ierror)
    if (ierror .ne. MPI_SUCCESS) THEN
       write(*,*) "MPI_Session_init failed"
       ERROR STOP
    end if

    call MPI_Session_get_num_psets(shandle, MPI_INFO_NULL, n_psets, ierror)
    IF (n_psets .lt. 2)  THEN
       write(*,*) "MPI_Session_get_num_psets didn't return at least 2 psets"
       ERROR STOP
    endif

!
!   Just get the second pset's length and name
!   Note that index values are zero-based, even in Fortran
!

    pset_len = 0
    call MPI_Session_get_nth_pset(shandle, MPI_INFO_NULL, 1, pset_len, &
                                  pset_name, ierror)

    allocate(character(len=pset_len)::pset_name)

    call MPI_Session_get_nth_pset(shandle, MPI_INFO_NULL, 1, pset_len, &
                                  pset_name, ierror)

!
!   create a group from the pset
!
    call MPI_Group_from_session_pset(shandle, pset_name, pgroup, ierror)
!
!   free the buffer used for the pset name
!
    deallocate(pset_name)

!
!   create a MPI communicator from the group
!
    call MPI_Comm_create_from_group(pgroup, "session_example",   &
                                            MPI_INFO_NULL,       &
                                            MPI_ERRORS_RETURN,   &
                                            pcomm, ierror)

    call MPI_Barrier(pcomm, ierror)
    if (ierror .ne. MPI_SUCCESS) then
        write(*,*) "Barrier call on communicator failed"
        ERROR STOP
    endif

    call MPI_Comm_free(pcomm, ierror)

    call MPI_Group_free(pgroup, ierror)

    call MPI_Session_finalize(shandle, ierror)
end program main

