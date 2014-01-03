#!/bin/sh
#
# This is the sloppiest test suite in the history of software engineering. It
# has bad coverage, it's barely functional, filled with time-driven tests. It's
# mostly useful to test if the essential of the program are in place and
# working when porting to other environments.
#

set -e
# set -v

. ./bootstrap.sh

run_test test_initial_setup
run_test test_generate_default_count
run_test test_generate_configured_count
run_test test_filter_standard_one_keyword
run_test test_filter_standard_two_keywords
run_test test_filter_regex_one_keyword
run_test test_filter_regex_two_keywords
run_test test_pager_timeout
run_test test_lock_retcode
run_test test_lock_output
run_test test_backup_file

print_summary

cleanup