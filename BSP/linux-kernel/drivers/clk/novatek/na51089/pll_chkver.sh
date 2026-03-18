#!/bin/bash

if test "$#" -ne 1; then
	echo "Usage: > $0 shipped object file"
	exit
fi

VERINFO=`strings $1 | grep "NT9856X_PLL" | sed 's/NT9856X_PLL#//g'`
if [ -z "${VERINFO:-}" ]; then
		echo $1
	else
		echo -e $1 ["$VERINFO"]
	fi