#!/bin/sh
#
# This is the sloppiest test suite in the history of software engineering. It
# has bad coverage, it's barely functional, filled with time-driven tests. It's
# mostly useful to test if the basics of the program are in place and working
# when porting to other environments.
#

# Simulates an editor writing four passwords.
if [ "$1" = "editor" ]; then
cat > $2 << EOF
strawberry red
raspberry red
blackberry black
grapefruit yellow
EOF
exit
fi

# Simulates an editor writing one password and taking 2 seconds to do so.
if [ "$1" = "sloweditor" ]; then
cat > $2 << EOF
roses red
EOF
sleep 2
exit
fi


# We're working in a fake GPG home
export GNUPGHOME="fake_gpg_home"
export HOME="$GNUPGHOME"
rm -rf $GNUPGHOME
mkdir -p $GNUPGHOME
chmod 700 $GNUPGHOME

echo -n "setup... "

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

# Create the key and grab its id
gpg --batch --no-options --gen-key test_gpg.batch 2>/dev/null
key_id=`gpg --list-keys 2>/dev/null | grep ^pub | sed 's/.*1024D.//;s/ .*//'`

# Create the config file with a fake editor
use_config() {
cat > test.config <<EOF
set gpg_path "/usr/local/bin/gpg"
set gpg_key_id "${key_id}"
set editor "`pwd`/test.sh $1"
set timeout 2
EOF
}


# First run to create a few password.
./mdp -c test.config -e 2>/dev/null

echo ok

echo -n "mdp -r red ... "
use_config editor
if ./mdp -c test.config -r red 2> /dev/null | diff - - << EOF
strawberry red
raspberry red
EOF
then
	echo SUCCESS
else
	echo FAIL
fi

echo -n "mdp -r berry black ... "
use_config editor
if ./mdp -c test.config -r red 2> /dev/null | diff - - << EOF
blackberry black
EOF
then
	echo SUCCESS
else
	echo FAIL
fi

echo -n "mdp berry black (timeout) ... "
use_config editor
start_ts=`date +%s`
./mdp -c test.config red 2>/dev/null 1>/dev/null
stop_ts=`date +%s`
delta=$((stop_ts - start_ts))
if [ $delta -ge 1 -o $delta -le 3 ]; then
	echo SUCCESS
else
	echo FAIL
fi

echo -n "mdp -e lock test ... "
use_config sloweditor
./mdp -c test.config -e 2>/dev/null 1>/dev/null &
sleep 0.1
if ./mdp -c test.config -e 2>/dev/null 1>/dev/null; then
	echo FAIL
else
	echo SUCCESS
fi
wait

echo -n "mdp -e lock test stderr ... "
use_config sloweditor
./mdp -c test.config -e 2>/dev/null 1>/dev/null &
OUTPUT=`./mdp -c test.config -e 2>&1`
if [ "$OUTPUT" = "mdp: locked (fake_gpg_home/.mdp/lock)" ]; then
	echo SUCCESS
else
	echo FAIL
fi
wait


# Cleanup.
rm -rf test.config test_gpg.batch $GNUPGHOME
