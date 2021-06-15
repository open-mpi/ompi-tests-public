#
# Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
#                         University Research and Technology
#                         Corporation.  All rights reserved.
# Copyright (c) 2004-2018 The University of Tennessee and The University
#                         of Tennessee Research Foundation.  All rights
#                         reserved.
# Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
#                         University of Stuttgart.  All rights reserved.
# Copyright (c) 2004-2005 The Regents of the University of California.
#                         All rights reserved.
# Copyright (c) 2006-2007 Sun Microsystems, Inc.  All rights reserved.
# Copyright (c) 2011-2016 Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2012      Los Alamos National Security, Inc.  All rights reserved.
# Copyright (c) 2013      Mellanox Technologies, Inc.  All rights reserved.
# Copyright (c) 2017-2018 Research Organization for Information Science
#                         and Technology (RIST). All rights reserved.
# Copyright (c) 2019      Triad National Security, LLC. All rights
#                         reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

# Use the Open MPI-provided wrapper compilers.

MPICC = mpicc
MPIFC = mpifort

# Example programs to build

EXAMPLES = \
	 sessions_ex1.o \
	 sessions_ex2 \
	 sessions_ex3 \
	 sessions_ex4 \
	 sessions_test \
	 sessions_test1 \
	 sessions_test2 \
	 sessions_test3 \
	 sessions_test4 \
	 sessions_test5 \
	 sessions_test6 \
	 sessions_test7 \
	 sessions_test8 \
	 sessions_test9 \
	 sessions_test10 \
	 sessions_test11 \
	 sessions_test12 \
	 sessions_test13 \
	 sessions_test14 \
	 sessions_test15 \
	 sessions_test16


# Default target.  Always build the C MPI examples.  Only build the
# others if we have the appropriate Open MPI / OpenSHMEM language
# bindings.

all: sessions_ex1.o sessions_ex2 sessions_test sessions_test1 sessions_test2 sessions_test3 sessions_test4 sessions_test5 \
	 sessions_test6 sessions_test7 sessions_test8 sessions_test9 sessions_test10 sessions_test11 sessions_test12 sessions_test13 \
	 sessions_test14 sessions_test15 sessions_test16
	@ if which ompi_info >/dev/null 2>&1 ; then \
		$(MAKE) mpi; \
	fi

mpi:
	@ if ompi_info --parsable | grep -q bindings:use_mpi_f08:yes >/dev/null; then \
		$(MAKE) sessions_ex3 ; \
	fi
	@ if ompi_info --parsable | egrep -q bindings:use_mpi:\"\?yes >/dev/null; then \
		$(MAKE) sessions_ex4 ; \
        fi


# The usual "clean" target

clean:
	rm -f $(EXAMPLES) *~ *.o

# Don't rely on default rules for the Fortran and Java examples

sessions_ex1.o: sessions_ex1.c
	$(MPICC) -c $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_ex2: sessions_ex2.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_ex3: sessions_ex3.f90
	$(MPIFC) $(FCFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_ex4: sessions_ex4.f90
	$(MPIFC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test: sessions_test.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test1: sessions_test1.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test2: sessions_test2.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test3: sessions_test3.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test4: sessions_test4.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test5: sessions_test5.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test6: sessions_test6.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test7: sessions_test7.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test8: sessions_test8.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test9: sessions_test9.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test10: sessions_test10.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test11: sessions_test11.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test12: sessions_test12.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test13: sessions_test13.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test14: sessions_test14.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test15: sessions_test15.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@
sessions_test16: sessions_test16.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) $? $(LDLIBS) -o $@


