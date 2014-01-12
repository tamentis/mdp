#!/bin/sh

# POSIX echo everywhere.
alias echo=/bin/echo

announce() {
	echo -n "$1... "
}

pass() {
	echo pass
}

skip() {
	echo skip
}

fail() {
	if [ -z "$1" ]; then
		echo fail
	else
		echo "fail - $1"
	fi
	exit 1
}

# $1 - file to test, typically test.stdout or test.stderr
assert_generic() {
	if diff $1 test.expected > test.diff; then
		return 0
	else
		echo "fail - $1 is not as expected"
		echo "--test.expected-----------"
		cat test.expected
		echo "--$1-------------"
		cat test.stdout
		echo "---------------------"
		exit 1
	fi
}

assert_stdout() {
	assert_generic test.stdout
}

assert_stderr() {
	assert_generic test.stderr
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
