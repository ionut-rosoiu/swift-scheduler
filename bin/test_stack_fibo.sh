#!/bin/bash

#
# Tests for fibo() with stack allocation for the frames and private data
# have a modified swift_scheduler -> swift_free() where no deallocation
# occurs because we had stack allocation
#

if test $# -lt 2; then
	echo "usage: $0 N NTHREADS"
	exit 0
fi

N=$1
NTHREADS=$2
TIME=/usr/bin/time

echo "N=$N NTHREADS=$NTHREADS"

$TIME -f "Time for   serial: real=%e user=%U sys=%S" ./test/fibo_serial $N
$TIME -f "Time for parallel (2 spawns on stack): real=%e user=%U sys=%S" ./test/fibo_stack_2spawns $NTHREADS $N

