#!/bin/bash -xe

# Final return value
FINAL_RTN=0

# Number of nodes - for accounting/verification purposes
# Default: 1
NUM_NODES=${CI_NUM_NODES:-1}

if [ "x" != "x${CI_HOSTFILE}" ] ; then
    ARG_HOSTFILE="--hostfile ${CI_HOSTFILE}"
else
    ARG_HOSTFILE=""
fi

_shutdown()
{
    # ---------------------------------------
    # Cleanup
    # ---------------------------------------

    exit $FINAL_RTN
}

# ---------------------------------------
# Run the test - Hostname
# ---------------------------------------
echo "=========================="
echo "Test: hostname"
echo "=========================="
mpirun ${ARG_HOSTFILE} --map-by ppr:5:node hostname 2>&1 | tee output-hn.txt

# ---------------------------------------
# Verify the results
# ---------------------------------------
ERRORS=`grep ERROR output-hn.txt | wc -l`
if [[ $ERRORS -ne 0 ]] ; then
    echo "ERROR: Error string detected in the output"
    FINAL_RTN=1
    _shutdown
fi

LINES=`wc -l output-hn.txt | awk '{print $1}'`
if [[ $LINES -ne $(( 5 * $NUM_NODES )) ]] ; then
    echo "ERROR: Incorrect number of lines of output"
    FINAL_RTN=2
    _shutdown
fi

if [ $FINAL_RTN == 0 ] ; then
    echo "Success - hostname"
fi


# ---------------------------------------
# Run the test - Hello World
# ---------------------------------------
echo "=========================="
echo "Test: Hello World"
echo "=========================="
mpirun ${ARG_HOSTFILE} --map-by ppr:5:node ./hello 2>&1 | tee output.txt

# ---------------------------------------
# Verify the results
# ---------------------------------------
ERRORS=`grep ERROR output.txt | wc -l`
if [[ $ERRORS -ne 0 ]] ; then
    echo "ERROR: Error string detected in the output"
    FINAL_RTN=1
    _shutdown
fi

LINES=`wc -l output.txt | awk '{print $1}'`
if [[ $LINES -ne $(( 5 * $NUM_NODES )) ]] ; then
    echo "ERROR: Incorrect number of lines of output"
    FINAL_RTN=2
    _shutdown
fi

if [ $FINAL_RTN == 0 ] ; then
    echo "Success - hello world"
fi

echo "=========================="
echo "Success"
echo "=========================="
_shutdown
