#!/bin/sh

set -e

base=`pwd`

find . -name "Makefile.src" | while read path; do
	directory=`dirname $path`
	# echo "-- going into $directory"
	cd $directory && make $1
	cd $base
done
