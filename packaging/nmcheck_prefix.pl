#!/usr/bin/env perl

# Copyright (c) 2020      IBM Corporation. All rights reserved.

sub identify_mpi_libraries_for_executable {
    my $exe = $_[0];
    my $cmd;
    my @tmp;
    my @line;
    my @libs;
    my $lib;
    my $dir;
    my $mpidir;
    my %libs_dir;

    %libs_dir = ();
    @tmp = split(/\n/, `ldd $exe 2>/dev/null`);
    for $line (@tmp) {
        if ($line =~ /^.* => *([^ (]*)/) {
            $lib = $1;
            if ($lib =~ m#(.*)/([^/]*)#) {
                $dir = $1;
                if (! -z "$dir" && -d "$dir") {
                    $dir = `cd $dir ; pwd`; chomp $dir;
                    $libs_dir{$2} = $dir;
                }
            }
        }
    }

    # One of these libraries should be libmpi.so or libmpi_something.so
    # and we'll keep every lib that's in the same directory as that lib
    $mpidir = '';
    for $lib (keys(%libs_dir)) {
        if ($lib =~ /libmpi_.*\./) {
            $mpidir = $libs_dir{$lib};
        }
    }
    for $lib (keys(%libs_dir)) {
        if ($lib =~ /libmpi\./) {
            $mpidir = $libs_dir{$lib};
        }
    }
    @libs = ();
    for $lib (keys(%libs_dir)) {
        if ($libs_dir{$lib} eq $mpidir) {
            push(@libs, "$mpidir/$lib");
        }
    }

    # Update: in SMPI at least we also put libevent in our <mpi_root>/lib
    # and even though those symbols could be a problem too, I lean toward
    # not caling them out here.
    @libs = grep(!/^libevent/, @libs);

    # I don't want to hard-code too much into this, but I'd like
    # to artificually make sure the fortran libraries are checked
    # and those wouldn't naturally show up for a C program.
    @tmp = split(/\n/, `cd $mpidir ; ls libmpi*_mpifh.so libmpi*_usempi.so libmpi_usempi_ignore_tkr.so libmpi_usempif08.so 2>/dev/null`);
    for $lib (@tmp) {
        if (-e "$mpidir/$lib") {
            push(@libs, "$mpidir/$lib");
        }
    }

    # print join("\n", @libs), "\n";

    return(@libs);
}

sub main {
    $x = @ARGV[0];
    if (! -e "$x") {
        print("One argument required: MPI executable\n");
        exit(-1);
    }
    @libs = identify_mpi_libraries_for_executable($x);

    print "Checking for bad symbol names:\n";
    $isbad = 0;
    for $lib (@libs) {
        print "*** checking $lib\n";
        check_lib_for_bad_exports($lib);
    }
    if ($isbad) { exit(-1); }
}

sub check_lib_for_bad_exports {
    my $lib = $_[0];
    my @symbols;
    my $s;

    @symbols = get_nm($lib, 'all');

    # grep to get rid of symbol prefixes that are considered acceptable,
    # leaving behind anything bad:
    @symbols = grep(!/^ompi_/i, @symbols);
    @symbols = grep(!/^ompix_/i, @symbols);
    @symbols = grep(!/^opal_/i, @symbols);
    @symbols = grep(!/^orte_/i, @symbols);
    @symbols = grep(!/^orted_/i, @symbols);
    @symbols = grep(!/^oshmem_/i, @symbols);
    @symbols = grep(!/^libnbc_/i, @symbols);
    @symbols = grep(!/^mpi_/i, @symbols);
    @symbols = grep(!/^mpix_/i, @symbols);
    @symbols = grep(!/^mpiext_/i, @symbols);
    @symbols = grep(!/^pmpi_/i, @symbols);
    @symbols = grep(!/^pmpix_/i, @symbols);
    @symbols = grep(!/^pmix_/i, @symbols);
    @symbols = grep(!/^pmix2x_/i, @symbols);
    @symbols = grep(!/^PMI_/i, @symbols);
    @symbols = grep(!/^PMI2_/i, @symbols);
    @symbols = grep(!/^MPIR_/, @symbols);
    @symbols = grep(!/^MPIX_/, @symbols);
    @symbols = grep(!/^mpidbg_dll_locations$/, @symbols);
    @symbols = grep(!/^mpimsgq_dll_locations$/, @symbols);
    @symbols = grep(!/^ompit_/i, @symbols);
    @symbols = grep(!/^ADIO_/i, @symbols);
    @symbols = grep(!/^ADIOI_/i, @symbols);
    @symbols = grep(!/^MPIO_/i, @symbols);
    @symbols = grep(!/^MPIOI_/i, @symbols);
    @symbols = grep(!/^MPIU_/i, @symbols);
    @symbols = grep(!/^NBC_/i, @symbols);  # seems sketchy to me
    @symbols = grep(!/^tm_/, @symbols);  # tempted to require ompi_ here
    @symbols = grep(!/^mca_/, @symbols);
    @symbols = grep(!/^smpi_/, @symbols);

    @symbols = grep(!/^_fini$/, @symbols);
    @symbols = grep(!/^_init$/, @symbols);
    @symbols = grep(!/^_edata$/, @symbols);
    @symbols = grep(!/^_end$/, @symbols);
    @symbols = grep(!/^__bss_start$/, @symbols);
    @symbols = grep(!/^__malloc_initialize_hook$/, @symbols);

    # Fortran compilers can apparently put some odd symbols in through
    # no fault of OMPI code.  I've at least seen "D &&N&mpi_types" created
    # by xlf from module mpi_types.  What we're trying to catch with this
    # testcase are OMPI bugs that need fixed, and I don't think OMPI is
    # likely to be creating such symbols through other means, so I'm
    # inclined to ignore any non-typical starting char as long as it's
    # in one of the fortran libs.
    if ($lib =~ /libmpi.*_usempi\./ ||
        $lib =~ /libmpi.*_usempi_ignore_tkr\./ ||
        $lib =~ /libmpi.*_usempif08\./)
    {
        @symbols = grep(!/^[^a-zA-Z0-9_]/, @symbols);

        @symbols = grep(!/^__mpi_/, @symbols);
        @symbols = grep(!/^_mpi_/, @symbols);
        @symbols = grep(!/^__pmpi_/, @symbols);
        @symbols = grep(!/^_pmpi_/, @symbols);
        @symbols = grep(!/^__mpiext_/, @symbols);
        @symbols = grep(!/^_mpiext_/, @symbols);
        @symbols = grep(!/^__ompi_/, @symbols);
        @symbols = grep(!/^_ompi_/, @symbols);
        @symbols = grep(!/^pompi_buffer_detach/, @symbols);
    }

    if ($lib =~ /libpmix\./) {
        # I'm only making this exception since the construct_dictionary.py
        # that creates dictionary.h is in a separate pmix repot.
        @symbols = grep(!/^dictionary$/, @symbols);
    }

    for $s (@symbols) {
        print "    [error]   $s\n";
        $isbad = 1;
    }
}

# get_nm /path/to/some/libfoo.so <func|wfunc|all>

sub get_nm {
    my $lib = $_[0];
    my $mode = $_[1];
    my $search_char;
    my @tmp;
    my @symbols;

    $search_char = "TWBCDVR";
    if ($mode eq 'func') { $search_char = "T"; }
    if ($mode eq 'wfunc') { $search_char = "W"; }

    @symbols = ();
    @tmp = split(/\n/, `nm $lib 2>/dev/null`);
    for $line (@tmp) {
        if ($line =~ /.* [$search_char] +([^ ]*)/) {
            push(@symbols, $1);
        }
    }

    @symbols = sort(@symbols);
    # print join("\n", @symbols), "\n";

    return(@symbols);
}

main();
