#!/bin/sh

set -e
# set -v

# Since we kinda need a minimum of terminal emulation for these to work, we
# skip them on Travis-CI.
if [ -n "$TRAVIS" ]; then
	echo "skipping functional tests on Travis"
	exit 0
fi

. ./_bootstrap.sh


for each in test_*.sh; do
	run_test ${each}
done

print_summary

cleanup
