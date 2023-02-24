#!/bin/bash

PROGS="prte prted prun mpirun timeout"

clean_files()
{
    FILES=("pmix-*" "core*" "openmpi-sessions-*" "pmix_dstor_*" "ompi.*" "prrte.*" )

    for fn in ${FILES[@]}; do
        find /tmp/ -maxdepth 1 \
            -user $USER -a \
            -name $fn \
            -exec rm -rf {} \;

        if [ -n "$TMPDIR" ] ; then
            find $TMPDIR -maxdepth 1 \
                -user $USER -a \
                -name $fn \
                -exec rm -rf {} \;
        fi
    done
}

killall -q ${PROGS} > /dev/null
clean_files
killall -q -9 ${PROGS} > /dev/null

exit 0


