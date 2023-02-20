#!/bin/bash

clean_server()
{
    SERVER=$1
    ITER=$2
    MAX=$3
    QUIET=$4

    SCRIPTDIR=$PWD/`dirname $0`/

    if [[ $QUIET == 0 ]] ; then 
        echo "Cleaning server ($ITER / $MAX): $SERVER"
    fi
    ssh -oBatchMode=yes ${SERVER} ${SCRIPTDIR}/cleanup-scrub-local.sh
}

if [[ "x" != "x$CI_HOSTFILE" && -f "$CI_HOSTFILE" ]] ; then
    ALLHOSTS=(`cat $CI_HOSTFILE | sort | uniq`)
else
    ALLHOSTS=(`hostname`)
fi
LEN=${#ALLHOSTS[@]}

# Use a background mode if running at scale
USE_BG=0
if [ $LEN -gt 10 ] ; then 
    USE_BG=1
fi

for (( i=0; i<${LEN}; i++ ));
do
    if [ $USE_BG == 1 ] ; then 
        if [ $(($i % 100)) == 0 ] ; then
            echo "| $i"
        else
            if [ $(($i % 10)) == 0 ] ; then
                echo -n "|"
            else
                echo -n "."
            fi
        fi
    fi

    if [ $USE_BG == 1 ] ; then
        clean_server ${ALLHOSTS[$i]} $i $LEN $USE_BG &
        sleep 0.25
    else
        clean_server ${ALLHOSTS[$i]} $i $LEN $USE_BG
        echo "-------------------------"
    fi
done

if [ $USE_BG == 1 ] ; then
    echo ""
    echo "------------------------- Waiting"
    wait
fi

echo "------------------------- Done"

exit 0
