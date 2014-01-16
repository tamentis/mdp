#!/bin/sh

set -e
# set -v

. ./_bootstrap.sh

for each in test_*.sh; do
	run_test ${each}
done

print_summary

cleanup
