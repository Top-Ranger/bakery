#!/bin/sh
cd $(dirname $(readlink -f $0))
cd dist/
export LD_LIBRARY_PATH="dist:$LD_LIBRARY_PATH"
for exec in ./tst_*; do
    $exec $@
done
