#!/bin/sh
#
# This file contains whatever can be abstracted from the daily maintenance of
# this test suite.
#

MDP=../../src/mdp
SELF=$0

if ! GPG=`which gpg`; then
	echo "error: couldn't find gpg in your PATH"
	exit 1
fi

test_count_passed=0
test_count_failed=0

alias echo=/bin/echo

if [ ! -f "$MDP" ]; then
	echo "error: you need to run make first"
	exit 1
fi

# Run mdp capturing stderr and using the test config file.
run_mdp() {
	$MDP -c test.config $@ 2>test.stderr
}
run_mdp_capture_stderr() {
	$MDP -c test.config $@ 2>&1
}


# Run a test unit
# $1 - testname
run_test() {
	filename=$1
	testname=${filename%%.sh}

	echo -n "$testname... "

	test_unit() {
		. ./$filename
	}

	if ! output=`test_unit`; then
		echo "error: unit failure"
		echo $output
		exit 1
	fi

	if [ "$output" = "pass" ]; then
		echo pass
		test_count_passed=$(( test_count_passed + 1 ))
		return 0
	else
		echo "FAILED! (output=$output)"
		test_count_failed=$(( test_count_failed + 1 ))
		return 1
	fi
}


# Return the md5 only.
# $1 - filename
get_md5() {
	# BSD md5?
	if md5_command=`which md5 2>/dev/null`; then
		md5 $1 | sed 's/.*[ \t]//g'
	# or GNU md5sum?
	elif md5_command=`which md5sum 2>/dev/null`; then
		md5sum $1 | sed 's/ .*//g'
	else
		echo "no md5 available"
		exit 1
	fi
}


# Return number of lines and bytes separated by a space.
get_lines_and_bytes() {
	wc -cl test.stdout | awk '{print $1, $2}'
}

# Remove all the files that could have been created before. Files are
# explicitely listed to avoid a glob to do something stupid.
cleanup() {
	# Test output
	rm -f test.config
	rm -f test_gpg.batch
	rm -f test.stdout
	rm -f test.stderr
	rm -f test.diff
	rm -f test.expected

	# Fake mdp home.
	rm -f fake_gpg_home/.mdp/passwords
	rm -f fake_gpg_home/.mdp/passwords.bak
	rm -f fake_gpg_home/.mdp/alternative
	rm -f fake_gpg_home/.mdp/alternative.bak
	rmdir fake_gpg_home/.mdp

	# GnuPG stuff, hopefully it doesn't vary by OS too much.
	rm -f fake_gpg_home/pubring.gpg
	rm -f fake_gpg_home/pubring.gpg~
	rm -f fake_gpg_home/random_seed
	rm -f fake_gpg_home/secring.gpg
	rm -f fake_gpg_home/trustdb.gpg
	rm -f fake_gpg_home/trustdb.gpg
	rmdir fake_gpg_home
}

# Makes sure an output from stdout matches expectations.
assert_stdout() {
	if diff test.stdout test.expected > test.diff; then
		echo pass
	fi
}

# Makes sure an output from stderr matches expectations.
assert_stderr() {
	if diff test.stderr test.expected > test.diff; then
		echo pass
	fi
}

# Ensures a given file exists.
# $1 - filepath
assert_file_exists() {
	if [ -f "$1" ]; then
		echo pass
	fi
}

print_summary() {
	echo
	echo "passed: $test_count_passed ($test_count_failed failed)"
}


# We're working in a fake GPG home
export GNUPGHOME="fake_gpg_home"
export HOME="fake_gpg_home"
rm -rf fake_gpg_home
mkdir -p fake_gpg_home
chmod 700 fake_gpg_home
passfile="fake_gpg_home/.mdp/passwords"
lockfile="fake_gpg_home/.mdp/lock"

# Create the batch to generate the key
cat 2> /dev/null > test_gpg.batch <<EOF
	Key-Type: DSA
	Key-Length: 1024
	Subkey-Type: ELG-E
	Subkey-Length: 1024
	Name-Real: mdp test suite
	Name-Email: mdp_test_suite@tamentis.com
	Expire-Date: 0
	# Passphrase: abc
	%commit
EOF

# Create the key and grab its id. Do not use --quick-random at home, it makes
# really shitty keys ;)
$GPG --batch --no-options --quick-random --gen-key test_gpg.batch 2>/dev/null
key_id=`$GPG --list-keys 2>/dev/null | grep ^pub | sed 's/.*1024D.//;s/ .*//'`

# Create the config file with a fake editor
# $1 - editor mode
use_config() {
cat > test.config <<EOF
set gpg_path "$GPG"
set gpg_key_id "${key_id}"
set editor "./_fake_editor.sh $1"
set timeout 3
EOF
}
