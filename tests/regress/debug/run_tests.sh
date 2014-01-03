#!/bin/sh

date=`date +"%Y-%m-%d %H:%M:%S"`

$1 something 2>&1 | awk '{print $5}' > output
echo "yup--something--" > expected

if diff output expected; then
	echo pass
else
	echo fail
fi
