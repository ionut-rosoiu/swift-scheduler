#!/bin/bash

if test $# -lt 2; then
	echo "usage: $0 N NTHREADS"
	exit 0
fi

N=$1
NTHREADS=$2
TIME=/usr/bin/time

echo "N=$N NTHREADS=$NTHREADS"

$TIME -f "Time for   serial: real=%e user=%U sys=%S" ./test/fibo_serial $N
$TIME -f "Time for parallel (3 spawns): real=%e user=%U sys=%S" ./test/fibo $NTHREADS $N
$TIME -f "Time for parallel (2 spawns): real=%e user=%U sys=%s" ./test/fibo_2spawn $NTHREADS $N

