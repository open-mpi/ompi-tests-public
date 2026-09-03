cmake_minimum_required(VERSION 3.10)
project(filetests C)

# Allow the user to specify the MPI installation path.
# Usage: cmake -DMPI_HOME=/path/to/mpi ..
if(DEFINED MPI_HOME)
    set(MPI_C_COMPILER "${MPI_HOME}/bin/mpicc")
    set(MPI_Fortran_COMPILER "${MPI_HOME}/bin/mpifort")
endif()

find_package(MPI REQUIRED COMPONENTS C)

# Build mode: set BUILD_COMBINED=ON (default) to build all tests into a single
# executable, or OFF to build each test as its own executable.
option(BUILD_COMBINED "Build all tests into a single 'filetest' executable" ON)

set(COMMON_SOURCES common.c)

if(BUILD_COMBINED)
    # All sources compiled with -DGLOBAL, entry point is filetest.c
    add_executable(filetest
        filetest.c
        open.c
        cart_comm_open.c
        write.c
        read.c
        fileview.c
        write_all.c
        write_all_2D.c
        read_all.c
        write_shared.c
        read_shared.c
        iwrite.c
        iread.c
        info.c
        utils.c
        atomicity.c
        common.c
    )
    target_compile_definitions(filetest PRIVATE GLOBAL)
    target_link_libraries(filetest PRIVATE MPI::MPI_C)
    target_include_directories(filetest PRIVATE ${MPI_C_INCLUDE_DIRS})
else()
    # Individual executables — each linked against common.c as needed

    add_executable(open open.c cart_comm_open.c)
    target_link_libraries(open PRIVATE MPI::MPI_C)
    target_include_directories(open PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(cart_open cart_open_main.c cart_comm_open.c)
    target_link_libraries(cart_open PRIVATE MPI::MPI_C)
    target_include_directories(cart_open PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(write write.c ${COMMON_SOURCES})
    target_link_libraries(write PRIVATE MPI::MPI_C)
    target_include_directories(write PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(iwrite iwrite.c ${COMMON_SOURCES})
    target_link_libraries(iwrite PRIVATE MPI::MPI_C)
    target_include_directories(iwrite PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(read read.c ${COMMON_SOURCES})
    target_link_libraries(read PRIVATE MPI::MPI_C)
    target_include_directories(read PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(iread iread.c ${COMMON_SOURCES})
    target_link_libraries(iread PRIVATE MPI::MPI_C)
    target_include_directories(iread PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(fileview fileview.c ${COMMON_SOURCES})
    target_link_libraries(fileview PRIVATE MPI::MPI_C)
    target_include_directories(fileview PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(write_all write_all.c write_all_2D.c ${COMMON_SOURCES})
    target_link_libraries(write_all PRIVATE MPI::MPI_C)
    target_include_directories(write_all PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(read_all read_all.c ${COMMON_SOURCES})
    target_link_libraries(read_all PRIVATE MPI::MPI_C)
    target_include_directories(read_all PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(write_shared write_shared.c ${COMMON_SOURCES})
    target_link_libraries(write_shared PRIVATE MPI::MPI_C)
    target_include_directories(write_shared PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(read_shared read_shared.c ${COMMON_SOURCES})
    target_link_libraries(read_shared PRIVATE MPI::MPI_C)
    target_include_directories(read_shared PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(info info.c ${COMMON_SOURCES})
    target_link_libraries(info PRIVATE MPI::MPI_C)
    target_include_directories(info PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(atomicity atomicity.c ${COMMON_SOURCES})
    target_link_libraries(atomicity PRIVATE MPI::MPI_C)
    target_include_directories(atomicity PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(utils utils.c ${COMMON_SOURCES})
    target_link_libraries(utils PRIVATE MPI::MPI_C)
    target_include_directories(utils PRIVATE ${MPI_C_INCLUDE_DIRS})

    add_executable(mt_open mt_open.c)
    target_link_libraries(mt_open PRIVATE MPI::MPI_C)
    target_include_directories(mt_open PRIVATE ${MPI_C_INCLUDE_DIRS})

    # mt_write uses SL_read from common.c (original Makefile was missing this)
    add_executable(mt_write mt_write.c ${COMMON_SOURCES})
    target_link_libraries(mt_write PRIVATE MPI::MPI_C)
    target_include_directories(mt_write PRIVATE ${MPI_C_INCLUDE_DIRS})
endif()
