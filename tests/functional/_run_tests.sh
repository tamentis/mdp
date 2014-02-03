#!/bin/sh

set -e
# set -v

. ./_bootstrap.sh

if [ -n "$1" ]; then
	while [ -n "$1" ]; do
		run_test $1
		shift
	done
else
	for each in test_*.sh; do
		run_test ${each}
	done

	print_summary
fi

cleanup
