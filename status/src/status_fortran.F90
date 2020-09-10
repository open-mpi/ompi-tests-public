!
! Copyright (C) 2020 Cisco Systems, Inc.
!
! $COPYRIGHT$
!
! See the README.md for more information

program main
  use mpi_f08
  implicit none

  ! Interface to the C routine we're going to call from here in
  ! Fortran
  interface
     subroutine test_c_functions(f08_status, f08_tag, f_status, f_tag, count, &
          f_mpi_status_size, f_mpi_source, f_mpi_tag, f_mpi_error) &
          BIND(C, name="test_c_functions")
       use mpi_f08
       implicit none
       type(MPI_Status), intent(in) :: f08_status
       integer, intent(in) :: f08_tag
       integer, dimension(MPI_STATUS_SIZE), intent(in) :: f_status
       integer, intent(in) :: f_tag
       integer, intent(in) :: count
       integer, intent(in) :: f_mpi_status_size, f_mpi_source, f_mpi_tag, f_mpi_error
     end subroutine test_c_functions
  end interface

  type(MPI_Status) :: f08_status
  integer, dimension(MPI_STATUS_SIZE) :: f_status
  integer :: f08_tag
  integer :: f_tag
  integer :: count
  integer :: f_true, f_false

  call MPI_Init()

  f08_tag = 111
  f_tag = 222
  ! An array of this many integers is 2GB in length
  count = 536870912
  ! This puts the array of integers over 2GB in length
  count = count + 10

  call generate_f08_status(f08_status, f08_tag, count)
  call generate_f_status(f_status, f_tag, count)

  call test_fortran_functions(f08_status, f08_tag, f_status, f_tag, count)

  ! NOTE: This calls a C function.
  ! NOTE: We are calling the C functions with "cancelled" set to true
  ! on both the f08 and f statuses!
  call test_c_functions(f08_status, f08_tag, f_status, f_tag, count, &
       MPI_STATUS_SIZE, MPI_SOURCE, MPI_TAG, MPI_ERROR)

  call MPI_Finalize()

end program main

!==================================================================

subroutine test_fortran_functions(f08_status, f08_tag, f_status, f_tag, count)
  use mpi_f08
  implicit none
  type(MPI_Status), intent(inout) :: f08_status
  integer, intent(in) :: f08_tag
  integer, dimension(MPI_STATUS_SIZE), intent(inout) :: f_status
  integer, intent(in) :: f_tag
  integer, intent(in) :: count

  print *, 'Testing Fortran functions...'

  ! Test MPI_Status_f082f (in Fortran)
  !
  ! These subroutines are called in deliberate order: the _mpif08
  ! subroutine *before* the _mpi subroutine.  This is because the
  ! _mpif08 subroutine can called MPI_Set_cancelled() on the F08
  ! status.  So we know for 100% sure the cancelled value of the
  ! status is when leaving the _mpif08 function, and check for that
  ! value in the _mpi subroutine.
  call test_fortran_f082f_mpif08(f08_status, f08_tag, count)
  call test_fortran_f082f_mpi(f08_status, f08_tag, count)

  ! Test MPI_Status_f2f08 (in Fortran)
  !
  ! Analogous to above, call the _mpi subroutine first so that it can
  ! call MPI_Set_cancelled() on the f_status, and therefore we can
  ! know for 100% what the cancelled value of that status is when
  ! calling the _mpif08 subroutine..
  call test_fortran_f2f08_mpi(f_status, f_tag, count)
  call test_fortran_f2f08_mpif08(f_status, f_tag, count)
end subroutine test_fortran_functions

!==================================================================

subroutine generate_f08_status(status, tag, count)
  use mpi_f08
  implicit none
  type(MPI_Status), intent(out) :: status
  integer, intent(in) :: tag
  integer, intent(in) :: count
  integer :: rank
  integer :: sendbuf, recvbuf
  type(MPI_Request), dimension(2) :: requests
  type(MPI_Status), dimension(2) :: statuses

  ! Create a status the "normal" way
  !
  ! Use a non-blocking send/receive to ourselves so that we can use an
  ! array WAIT function so that the MPI_ERROR element will be set in
  ! the resulting status.

  call MPI_Comm_rank(MPI_COMM_WORLD, rank)

  sendbuf = 123
  call MPI_Irecv(recvbuf, 1, MPI_INTEGER, rank, tag, MPI_COMM_WORLD, &
       requests(1))
  call MPI_Isend(sendbuf, 1, MPI_INTEGER, rank, tag, MPI_COMM_WORLD, &
       requests(2))
  call MPI_Waitall(2, requests, statuses)

  ! Copy the resulting receive status to our output status
  status = statuses(1)

  ! Now set some slightly different values in the status that results
  ! in a very large count (larger than can be represented by 32 bits)
  call MPI_Status_set_cancelled(status, .false.)
  call MPI_Status_set_elements(status, MPI_INTEGER, count)
end subroutine generate_f08_status

!==================================================================

subroutine generate_f_status(status, tag, count)
  use mpi
  implicit none
  integer, dimension(MPI_STATUS_SIZE), intent(out) :: status
  integer, intent(in) :: tag
  integer, intent(in) :: count
  integer :: rank, ierr, sendbuf, recvbuf
  integer, dimension(2) :: requests
  integer, dimension(MPI_STATUS_SIZE, 2) :: statuses

  ! Create a status the "normal" way
  !
  ! Use a non-blocking send/receive to ourselves so that we can use an
  ! array WAIT function so that the MPI_ERROR element will be set in
  ! the resulting status.

  call MPI_Comm_rank(MPI_COMM_WORLD, rank, ierr)

  sendbuf = 456
  call MPI_Irecv(recvbuf, 1, MPI_INTEGER, rank, tag, MPI_COMM_WORLD, &
       requests(1), ierr)
  call MPI_Isend(sendbuf, 1, MPI_INTEGER, rank, tag, MPI_COMM_WORLD, &
       requests(2), ierr)
  call MPI_Waitall(2, requests, statuses, ierr)

  ! Copy the resulting receive status to our output status
  status = statuses(:, 1)

  ! Now set some slightly different values in the status that results
  ! in a very large count (larger than can be represented by 32 bits)
  call MPI_Status_set_cancelled(status, .false., ierr)
  call MPI_Status_set_elements(status, MPI_INTEGER, count, ierr)
end subroutine generate_f_status

!==================================================================

subroutine check_integer(a, b, msg)
  implicit none
  integer, intent(in) :: a, b
  character(len=*), intent(in) :: msg

  if (a .ne. b) then
     print *, "Error: ", msg, ": ", a, "<>", b
     stop
  end if
end subroutine check_integer

!==================================================================

! This subroutine is called from C.
! We have to translate the 1/0 from C into a Fortran logical.  Sigh.
subroutine check_cancelled_f_wrapper(f_status, expected_value, msg) &
     BIND(C, name="check_cancelled_f_wrapper")
  use mpi
  implicit none
  integer, dimension(MPI_STATUS_SIZE), intent(in) :: f_status
  integer, intent(in) :: expected_value
  character(len=1), intent(in) :: msg

  if (expected_value .eq. 1) then
     call check_cancelled_f(f_status, .true., msg)
  else
     call check_cancelled_f(f_status, .false., msg)
  end if
end subroutine check_cancelled_f_wrapper

subroutine check_cancelled_f(f_status, expected_value, msg) &
     BIND(C, name="check_cancelled_f")
  use mpi
  implicit none
  integer, dimension(MPI_STATUS_SIZE), intent(in) :: f_status
  logical, intent(in) :: expected_value
  character(len=1), intent(in) :: msg
  integer :: ierr
  logical :: value

  call MPI_Test_cancelled(f_status, value, ierr)
  if (value .neqv. expected_value) then
     print *, "Error: ", msg, ": ", value, "<>", expected_value
     stop
  end if
end subroutine check_cancelled_f

!------------------------------------------------------------------

! This subroutine is called from both Fortran and C
subroutine check_count_f(f_status, expected_count, msg) &
     BIND(C, name="check_count_f")
  use mpi
  implicit none
  integer, dimension(MPI_STATUS_SIZE), intent(in) :: f_status
  integer, intent(in) :: expected_count
  character(len=1), intent(in) :: msg
  integer :: ierr
  integer :: count

  call MPI_Get_count(f_status, MPI_INTEGER, count, ierr)
  call check_integer(count, expected_count, msg)
end subroutine check_count_f

!------------------------------------------------------------------

subroutine test_fortran_f082f_mpi(f08_status, expected_tag, expected_count)
  use mpi
  implicit none
  type(MPI_Status), intent(inout) :: f08_status
  integer, intent(in) :: expected_tag
  integer, intent(in) :: expected_count

  integer, dimension(MPI_STATUS_SIZE) :: f_status
  integer :: rank, ierr

  print *, 'Testing Fortran MPI_Status_f082f (mpi module)'

  call MPI_Comm_rank(MPI_COMM_WORLD, rank, ierr)

  ! Coming in to this routine, the f08 status will be set to .true.
  call MPI_Status_f082f(f08_status, f_status, ierr)

  call check_integer(f_status(MPI_SOURCE), rank, "f082f source 2")
  call check_integer(f_status(MPI_TAG), expected_tag, "f082f tag 2")
  call check_integer(f_status(MPI_ERROR), MPI_SUCCESS, "f08f2 error 2")
  call check_cancelled_f(f_status, .true., "f082f .true.")
  call check_count_f(f_status, expected_count, "f082f count")
end subroutine test_fortran_f082f_mpi

!------------------------------------------------------------------

subroutine test_fortran_f082f_mpif08(f08_status, expected_tag, expected_count)
  use mpi_f08
  implicit none
  type(MPI_Status), intent(inout) :: f08_status
  integer, intent(in) :: expected_tag
  integer, intent(in) :: expected_count

  integer, dimension(MPI_STATUS_SIZE) :: f_status
  integer :: rank

  print *, 'Testing Fortran MPI_Status_f082f (mpi_f08 module)'

  call MPI_Comm_rank(MPI_COMM_WORLD, rank)

  ! Set the canceled status to .false. and check everything
  call MPI_Status_set_cancelled(f08_status, .false.)
  call MPI_Status_f082f(f08_status, f_status)

  call check_integer(f_status(MPI_SOURCE), rank, "f082f source")
  call check_integer(f_status(MPI_TAG), expected_tag, "f082f tag")
  call check_integer(f_status(MPI_ERROR), MPI_SUCCESS, "f082f error")
  call check_cancelled_f(f_status, .false., "f082f .false.")
  call check_count_f(f_status, expected_count, "f082f count")

  ! Set the canceled status to .true. and check everything
  call MPI_Status_set_cancelled(f08_status, .true.)
  call MPI_Status_f082f(f08_status, f_status)

  call check_integer(f_status(MPI_SOURCE), rank, "f082f source 2")
  call check_integer(f_status(MPI_TAG), expected_tag, "f082f tag 2")
  call check_integer(f_status(MPI_ERROR), MPI_SUCCESS, "f08f2 error 2")
  call check_cancelled_f(f_status, .true., "f082f .true.")
  call check_count_f(f_status, expected_count, "f082f count")
end subroutine test_fortran_f082f_mpif08

!==================================================================

! This subroutine is called from C.
! We have to translate the 1/0 from C into a Fortran logical.  Sigh.
subroutine check_cancelled_f08_wrapper(f08_status, expected_value, msg) &
     BIND(C, name="check_cancelled_f08_wrapper")
  use mpi_f08
  implicit none
  type(MPI_Status), intent(in) :: f08_status
  integer, intent(in) :: expected_value
  character(len=1), intent(in) :: msg

  if (expected_value .eq. 1) then
     call check_cancelled_f08(f08_status, .true., msg)
  else
     call check_cancelled_f08(f08_status, .false., msg)
   end if
end subroutine check_cancelled_f08_wrapper

subroutine check_cancelled_f08(f08_status, expected_value, msg)
  use mpi_f08
  implicit none
  type(MPI_Status), intent(in) :: f08_status
  logical, intent(in) :: expected_value
  character(len=1), intent(in) :: msg
  integer :: ierr
  logical :: value

  call MPI_Test_cancelled(f08_status, value, ierr)
  if (value .neqv. expected_value) then
     print *, "Error: ", msg, ": ", value, "<>", expected_value
     stop
  end if
end subroutine check_cancelled_f08

!------------------------------------------------------------------

! This subroutine is called from both Fortran and C
subroutine check_count_f08(f08_status, expected_count, msg) &
     BIND(C, name="check_count_f08")
  use mpi_f08
  implicit none
  type(MPI_Status), intent(in) :: f08_status
  integer, intent(in) :: expected_count
  character(len=1), intent(in) :: msg
  integer :: ierr
  integer :: count

  call MPI_Get_count(f08_status, MPI_INTEGER, count, ierr)
  call check_integer(count, expected_count, msg)
end subroutine check_count_f08

!------------------------------------------------------------------

subroutine test_fortran_f2f08_mpi(f_status, expected_tag, expected_count)
  use mpi
  implicit none
  integer, dimension(MPI_STATUS_SIZE), intent(inout) :: f_status
  integer, intent(in) :: expected_tag
  integer, intent(in) :: expected_count

  type(MPI_Status) :: f08_status
  integer :: rank, ierr

  print *, 'Testing Fortran MPI_Status_f2f08 (mpi module)'

  call MPI_Comm_rank(MPI_COMM_WORLD, rank, ierr)

  ! Set the canceled status to .false. and check everything
  call MPI_Status_set_cancelled(f_status, .false., ierr)
  call MPI_Status_f2f08(f_status, f08_status, ierr)

  call check_integer(f08_status%MPI_SOURCE, rank, "f2f08 source")
  call check_integer(f08_status%MPI_TAG, expected_tag, "f2f08 tag")
  call check_integer(f08_status%MPI_ERROR, MPI_SUCCESS, "f2f08 error")
  call check_cancelled_f08(f08_status, .false., "f2f08 .false.")
  call check_count_f08(f08_status, expected_count, "f2f08 count")

  ! Set the canceled status to .true. and check everything
  call MPI_Status_set_cancelled(f_status, .true., ierr)
  call MPI_Status_f2f08(f_status, f08_status, ierr)

  call check_integer(f08_status%MPI_SOURCE, rank, "f2f08 source 2")
  call check_integer(f08_status%MPI_TAG, expected_tag, "f2f08 tag 2")
  call check_integer(f08_status%MPI_ERROR, MPI_SUCCESS, "f08f2 error 2")
  call check_cancelled_f08(f08_status, .true., "f2f08 .true.")
  call check_count_f08(f08_status, expected_count, "f2f08 count")
end subroutine test_fortran_f2f08_mpi

!------------------------------------------------------------------

subroutine test_fortran_f2f08_mpif08(f_status, expected_tag, expected_count)
  use mpi_f08
  implicit none
  integer, dimension(MPI_STATUS_SIZE), intent(inout) :: f_status
  integer, intent(in) :: expected_tag
  integer, intent(in) :: expected_count

  type(MPI_Status) :: f08_status
  integer :: rank

  print *, 'Testing Fortran MPI_Status_f2f08 (mpi_f08 module)'

  call MPI_Comm_rank(MPI_COMM_WORLD, rank)

  ! Coming in to this routine, the f08 status will be set to .true.
  call MPI_Status_f2f08(f_status, f08_status)

  call check_integer(f08_status%MPI_SOURCE, rank, "f2f08 source 2")
  call check_integer(f08_status%MPI_TAG, expected_tag, "f2f08 tag 2")
  call check_integer(f08_status%MPI_ERROR, MPI_SUCCESS, "f08f2 error 2")
  call check_cancelled_f08(f08_status, .true., "f2f08 .true.")
  call check_count_f08(f08_status, expected_count, "f2f08 count")
end subroutine test_fortran_f2f08_mpif08
