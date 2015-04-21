#!/bin/sh
#
# _run_tests.sh without parameters will run all the tests in the current
# directory, you may also specify as many test files as needed as command-line
# parameters.
#

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
