#!/bin/sh

# POSIX echo everywhere.
alias echo=/bin/echo

announce() {
	echo -n "$1... "
}

pass() {
	echo pass
}

fail() {
	if [ -z "$1" ]; then
		echo fail
	else
		echo "fail - $1"
	fi
	exit 1
}

assert_output() {
	if diff output expected > diff; then
		return 0
	else
		echo "fail - output is not as expected"
		echo "--expected-----------"
		cat expected
		echo "--output-------------"
		cat output
		echo "---------------------"
		exit 1
	fi
}

assert_file_exists() {
	if [ -f "$1" ]; then
		return 0
	else
		fail "file $1 does not exist"
	fi
}

assert_file_missing() {
	if [ ! -f "$1" ]; then
		return 0
	else
		fail "file $1 should not exists"
	fi
}

assert_retcode_failure() {
	if [ "$1" != "0" ]; then
		return 0
	else
		fail "not a failure return code: $1"
	fi
}

assert_retcode_success() {
	if [ "$1" = "0" ]; then
		return 0
	else
		fail "not a success return code: $1"
	fi
}
