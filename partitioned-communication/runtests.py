# Copyright 2024 National Technology & Engineering Solutions of Sandia, LLC    
# (NTESS).  Under the terms of Contract DE-NA0003525 with NTESS, the U.S.      
# Government retains certain rights in this software.

# A simple python script for running unit tests and checking their outputs
#
# The `tests` dict (below) specifies which tests are to be run and how the 
# stderr output of each test is to be evaluated. Tests can pass because 
# (i) an expected string was found in the output (or, optionally, on a specific 
# line of the output), or (ii) the test was expected to timeout and it did.
#
# Some tests are written to elicit an error message from the MPI implementation. Because it may not 
# be known a priori what this message is, the text variable TBD_ERROR is used 
# as the matching string, with the expectation it will be replaced with the 
# actual error message when it is available. Below, tests that contain TBD_ERROR 
# are tests that fail because no error is reported by OMPI.
#
# All tests are written to use stderr, and assume OMPI error messages will appear on stderr; stdout is ignored.
#
# The script assumes if a segmentation fault occurs in the MPI implementation, it is an error. 
# Segmentation faults can generate stderr outputs that are not readable by Python in text mode (only 
# in binary mode). Therefore, the script tests if the output is readable in text mode; if it is not, it 
# reports the test as failing and notes the file was not readable.
#
#

import argparse
import os
import subprocess
import sys
import threading
import time

# Use the following as placeholder text for tests designed to elicit an error message. 
# When the specific error message is known, replace.
TBD_ERROR = "UNKNOWN ERROR MESSAGE EXPECTED"

# For tests designed to elicit a timeout, use the following matching text:
TIMEOUT_EXPECTED = "__TIMEOUT_EXPECTED__"

# The following variables are default text for generating the report
TIMEOUT_EXPECTED_AND_FOUND = "Timeout expected and did occur"
TIMEOUT_EXPECTED_AND_NOT_FOUND = "A timeout was expected but the test terminated for some other reason."
TIMEOUT_UNEXPECTED_AND_FOUND = "No timeout was expected but test terminated due to timeout"
TIMEOUT_UNEXPECTED_AND_NOT_FOUND = "This outcome should not arise"

# subdirectory where tests are located relative to this script
BIN = "./bin/"

# Dict of tests to run along with the string that, if found in the ouput, means the test passes.
#
# The tuple is the string to be matched, followed by the line on which it is expected. Use "*" to allow the 
# matching string to appear on any line.
#
# TBD_ERROR is used as the expected string when a test is supposed to elicit an error but we do not 
# have a proper output string to test for (e.g., if the test fails to error).
#
# If a timeout is the expected outcome, then use the TIMEOUT_EXPECTED variable for the match string.
#
tests = {
          "test_cancel0.x": (TBD_ERROR,"*"),
          "test_datatype0.x": ("END", "1"), 
          "test_datatype1.x": ("END", "1"), 
          "test_datatype2.x": ("END", "1"), 
          "test_datatype3.x": ("END", "1"), 
          "test_datatype4.x": ("END", "1"), 
          "test_datatype5.x": ("END", "1"), 
          "test_example1a.x": ("END", "1"),
          "test_example1b.x": ("END", "1"),
          "test_example2.x": ("END", "1"),
          "test_example3a.x": ("END", "1"),
          "test_example3b.x": ("END", "1"),
          "test_example3c.x": ("END", "1"),
          "test_free0.x": (TBD_ERROR, "*"),
          "test_init0.x": (TIMEOUT_EXPECTED, "*"),
          "test_init1.x": (TIMEOUT_EXPECTED, "*"),
          "test_init2.x": (TIMEOUT_EXPECTED, "*"),
          "test_local0.x" : ("END", "1"),
          "test_local1.x" : ("END", "1"),
          "test_numparts0.x" : ("END", "1"),
          "test_numparts1.x" : ("END", "1"),
          "test_order0.x" : ("END", "1"),
          "test_parrived0.x" : ("END", "*"),
          "test_parrived1.x" : ("END", "*"),
          "test_parrived2.x" : ("*** An error occurred in MPI_Parrived", "*"),
          "test_partitions0.x" : (TBD_ERROR, "*"),
          "test_partitions1.x" : (TBD_ERROR, "*"),
          "test_partitions2.x" : (TBD_ERROR, "*"),
          "test_partitions3.x" : (TBD_ERROR, "*"),
          "test_pready0.x" : (TBD_ERROR, "*"),
          "test_pready1.x" : (TBD_ERROR, "*"),
          "test_pready2.x" : (TBD_ERROR, "*"),
          "test_pready3.x" : (TBD_ERROR, "*"),
          "test_pready4.x" : ("MPI_ERR_REQUEST: invalid request", "*"),
          "test_pready_list0.x": ("END", "1"),
          "test_pready_list1.x": (TBD_ERROR, "*"),
          "test_pready_range0.x": ("END", "1"),
          "test_startall0.x" : ("END", "*"),
          "test_state0.x" : (TBD_ERROR, "*"),
          "test_wildcard0.x" : (TBD_ERROR, "*"),
          "test_wildcard1.x" : (TBD_ERROR, "*"),
          "test_zerocount0.x" : ("END", "*"),
          "test_zerocount1.x" : ("END", "*")
        }

# for colored verbose output
class txt_colors:
    PASS = '\033[92m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

# test output is directed here
out_stderr = ".stderr"
out_stdout = ".stdout"

# prefix of output directory (date/time will be appended)
outdir = "results_"

# name of report file
reportfn = "report.txt"

# for timeout
timeout_thread_abort = False
timeout_occurred = False
DEFAULT_TIMEOUT_SECONDS = 25

# for stats
passed = 0
failed = 0

def error(msg):
    print("ERROR: {0}".format(msg), file=sys.stderr)
    exit(1)

# if the test passed
def report_success(outfile, test, msg=""):
    global passed
    passed += 1
    print("{0} : passed : {1}".format(test, msg), file=outfile)

# if the test failed
def report_failure(outfile, test, msg=""):
    global failed
    failed += 1
    print("{0} : FAILED : {1}".format(test, msg), file=outfile)

def get_process_id(name):
    child = subprocess.Popen(['pgrep', '-f', name], stdout=subprocess.PIPE, shell=False)
    response = child.communicate()[0]
    return [int(pid) for pid in response.split()]

# function for timeout thread
def timeout_function(seconds, t):
    global timeout_occurred
    timeout_occurred = False
    count = 0
    pid = get_process_id(t)
    while (not pid) and (count < 5):
        pid = get_process_id(t)
    t_start = int(time.time())
    while ((int(time.time()) - t_start) < seconds):
        if timeout_thread_abort:
            return
    timeout_occurred = True
    subprocess.run("kill -9 {0}".format(pid[0]), stderr=outfile_stderr, stdout=subprocess.DEVNULL, shell=True)
    return

# check if output file to be checked has only text
# When a segfault occurs, it can generate outout that cannot be decoded to text
def file_is_readable(infn):
    try:
        infile = open(infn, "r")
    except:
        error("Could not open file {0}".format(infn))
    try:
        for line in infile:
            print("", end='')
    except:
        return False
    return True

# helper to avoid duplicating code
def verbose_pass_fail(test_passed):
    if test_passed:
        print(f"{txt_colors.PASS}PASSED{txt_colors.ENDC}", file=sys.stdout)
    else:
        print(f"{txt_colors.FAIL}FAILED{txt_colors.ENDC}", file=sys.stdout)


if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("-v", "--verbose", action="store_true", dest="verbose", help="Verbose mode")

    args = parser.parse_args()
    verbose = args.verbose

    tstr = time.strftime("%Y%m%d-%H%M%S")  
    outdir = outdir + tstr
    os.mkdir(outdir)
    outdir = "./" + outdir

    reportfn = outdir + "/" + reportfn
    try:
        reportfile = open(reportfn, "w")
    except:
        error("Could not open file {0} for writing A".format(reportfn))

    total_tests = len(tests)
    cur_test = 0

    # add some info to the report
    print("", file=reportfile)
    print("---- Tests start: {0} ----".format(tstr), file=reportfile)
    print("Tests (num = {0}):".format(total_tests), file=reportfile)
    for t in tests:
        print("     {0}".format(t), file=reportfile)
    print(file=reportfile)

    tt_start = time.time()

    for t in tests:
        cur_test += 1
        if verbose:
            print("{0} / {1} : Running test: {2} --> ".format(cur_test, total_tests, t),end='', file=sys.stdout, flush=True)
        command = "mpirun -np 2 --npernode 1 " + BIN + t
        tmp_out_stderr = outdir + "/" + t + out_stderr
        try:
            outfile_stderr = open(tmp_out_stderr, 'w')
        except:
            error("Could not open file {0} for writing B".format(tmp_out_stderr))

        timeout_thread_abort = False
        timeout_thread = threading.Thread(target=timeout_function, args=(DEFAULT_TIMEOUT_SECONDS,t,))

        timeout_thread.start()
        subprocess.run(command, stderr=outfile_stderr, stdout=subprocess.DEVNULL, shell=True)
        if not timeout_occurred:
            timeout_thread_abort = True
        outfile_stderr.close()

        match_string = tests[t][0]
        testline = tests[t][1]

        test_passed = False

        # Confirm the file can be read as text
        # If a segmentation fault occurs in the MPI implementation, it can generate contents that are 
        # only readable in binary mode, which this code cannot test. So if it is not readable, its an 
        # error.
        is_readable = file_is_readable(tmp_out_stderr)
        if not is_readable:
            report_failure(reportfile, t, "Output is not readable as text. Check .stderr output")
            if verbose:
                verbose_pass_fail(test_passed)
            continue

        testfile = open(tmp_out_stderr,'r')

        curline = 0
        foundline = False
        # timeout occurred but not expected
        if timeout_occurred and (match_string != TIMEOUT_EXPECTED): 
            report_failure(reportfile, t, TIMEOUT_UNEXPECTED_AND_FOUND)
            foundline = True # to avoid another failure message
        # timeout occured and expected
        elif timeout_occurred and (match_string == TIMEOUT_EXPECTED):
            report_success(reportfile, t, TIMEOUT_EXPECTED_AND_FOUND)
            foundline = True # to avoid another failure message
            test_passed = True
        # timeout did not occur but was expected
        elif (not timeout_occurred) and (match_string == TIMEOUT_EXPECTED):
            report_failure(reportfile, t, TIMEOUT_EXPECTED_AND_NOT_FOUND)
            foundline = True # to avoid another failure message
        elif testline == "*":
            for line in testfile:
                curline += 1
                line = line.rstrip()
                if match_string in line:
                    foundline = True
                    report_success(reportfile, t, "Match found on line {0}".format(curline))
                    test_passed = True
                    break
        else: # look only at specified line
            try:
                testline = int(testline)
            except:
                error("Line specified for test {0} is neither an int nor *")
            for line in testfile:
                curline += 1
                line = line.rstrip()
                if curline == testline:
                    foundline = True
                    if match_string in line:
                        report_success(reportfile, t, "Match found on line {0}".format(curline))
                        test_passed = True
                    else:
                        report_failure(reportfile, t, "On line {0}: Expected: {1} Found: {2}".format(testline, match_string, line))
                    break

        # If no match found, provide more information
        if not foundline:
            if testline == "*":
                report_failure(reportfile, t, "Expected text not found on any line. ({0})".format(match_string))
            else:
                report_failure(reportfile, t, "Expected to test on line {0}, but stderr output file has only {1} lines.".format(testline, curline))

        if verbose:
            verbose_pass_fail(test_passed)

        time.sleep(7)

    tt_end = time.time()

    print("", file=reportfile)
    print("", file=reportfile)
    if (total_tests != (passed + failed)):
        print("Note: total_tests (= {0}) did not equal passed + failed (= {1})".format(total_tests, passed, failed), file=reportfile)
    passed_total = float(passed)/float(total_tests)
    failed_total = float(failed)/float(total_tests)
    print("{0} of {1} tests passed ({2:.2f}%)".format(passed, total_tests, passed_total*100), file=reportfile)
    print("{0} of {1} tests failed ({2:.2f}%)".format(failed, total_tests, failed_total*100), file=reportfile)
    print("", file=reportfile)
    print("---- Tests end: {0} ----".format(time.strftime("%Y%m%d-%H%M%S")), file=reportfile)
    print("---- Total time required to run tests: {0:.2f} seconds".format(tt_end - tt_start), file=reportfile)
    reportfile.close()

    if verbose:
        print("{0} of {1} tests passed ({2:.2f}%)".format(passed, total_tests, passed_total*100), file=sys.stdout)
        print("{0} of {1} tests failed ({2:.2f}%)".format(failed, total_tests, failed_total*100), file=sys.stdout)

