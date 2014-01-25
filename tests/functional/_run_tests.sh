#!/bin/sh

set -e
# set -v

. ./_bootstrap.sh

if [ -n "$1" ]; then
	run_test $1
else
	for each in test_*.sh; do
		run_test ${each}
	done

	print_summary
fi

cleanup
