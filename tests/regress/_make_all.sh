#!/bin/sh

set -e

base=`pwd`

find . -type d -depth 1 | while read directory; do
	echo "-- going into $directory"
	cd $directory && make $1
	cd $base
done
