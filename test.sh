#!/bin/sh
#
# This is the sloppiest test suite in the history of software engineering. It
# has bad coverage, it's barely functional, filled with time-driven tests. It's
# mostly useful to test if the essential of the program are in place and
# working when porting to other environments.
#

MDP=src/mdp
GPG=`which gpg`


# Return the md5 only.
# $1 - filename
get_md5() {
	# Find ourself an md5 command.
	if ! md5_command=`which md5 2>/dev/null`; then
		md5_command=`which md5sum 2>/dev/null`
	fi

	$md5_command $1 | sed 's/.*[ \t]//g'
}


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

# Writes four other passwords.
if [ "$1" = "alt_editor" ]; then
cat > $2 << EOF
tiger feline with stripes
cat black
dog white
rat grey
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
passfile="$GNUPGHOME/.mdp/passwords"

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
$GPG --batch --no-options --gen-key test_gpg.batch 2>/dev/null
key_id=`$GPG --list-keys 2>/dev/null | grep ^pub | sed 's/.*1024D.//;s/ .*//'`

# Create the config file with a fake editor
use_config() {
cat > test.config <<EOF
set gpg_path "$GPG"
set gpg_key_id "${key_id}"
set editor "`pwd`/test.sh $1"
set timeout 3
EOF
}


# First run to create a few password. We test the return code of this since
# failing to create the initial password would cause this to die.
use_config editor
if $MDP -c test.config -e 2>/dev/null; then
	echo PASS
else
	echo FAIL
fi


# Filter on red and expect all the lines containing red in the output.
echo -n "mdp -r red ... "
use_config editor
$MDP -c test.config -r red 2> /dev/null > test.output
if diff test.output - << EOF
strawberry red
raspberry red
EOF
then
	echo PASS
else
	echo FAIL
fi


# Filter on two keywords, berry and black.
echo -n "mdp -r berry black ... "
use_config editor
$MDP -c test.config -r berry black 2> /dev/null > test.output
if diff test.output - << EOF
blackberry black
EOF
then
	echo PASS
else
	echo FAIL
fi


# Filter on one regex.
echo -n "test one regex (-E) ... "
use_config editor
$MDP -c test.config -r -E ^.....berry 2> /dev/null > test.output
if diff test.output - << EOF
strawberry red
blackberry black
EOF
then
	echo PASS
else
	echo FAIL
fi


# Filter on two regex.
echo -n "test two regex (-E) ... "
use_config editor
$MDP -c test.config -r -E berry 'red$' 2> /dev/null > test.output
if diff test.output - << EOF
strawberry red
raspberry red
EOF
then
	echo PASS
else
	echo FAIL
fi


# Filter on two keywords, test timeout setting (should pause for 3 seconds).
echo -n "mdp berry black (timeout) ... "
use_config editor
start_ts=`date +%s`
$MDP -c test.config red 2>/dev/null 1>/dev/null
stop_ts=`date +%s`
delta=$((stop_ts - start_ts))
if [ $delta -ge 2 ] || [ $delta -le 4 ]; then
	echo PASS
else
	echo FAIL
fi


# Make sure the locks works properly (mdp returns non-0).
echo -n "mdp -e lock test ... "
use_config sloweditor
$MDP -c test.config -e 2>/dev/null 1>/dev/null &
sleep 0.1
if ./mdp -c test.config -e 2>/dev/null 1>/dev/null; then
	echo FAIL
else
	echo PASS
fi
wait


# Test the output of mdp if locked.
echo -n "mdp -e lock test stderr ... "
use_config sloweditor
$MDP -c test.config -e 2>/dev/null 1>/dev/null &
OUTPUT=`$MDP -c test.config -e 2>&1`
if [ "$OUTPUT" = "mdp: locked (fake_gpg_home/.mdp/lock)" ]; then
	echo PASS
else
	echo FAIL
fi
wait


# Test the backup file.
echo -n "test backup file ... "
rm -f $passfile.bak
use_config editor
$MDP -c test.config -e 2>/dev/null 1>/dev/null
before_md5=`get_md5 $passfile`
if [ ! -f "$passfile.bak" ]; then
	echo "FAIL (bak file not found)"
else
	use_config alt_editor
	$MDP -c test.config -e 2>/dev/null 1>/dev/null
	after_md5=`get_md5 $passfile`
	bak_md5=`get_md5 $passfile.bak`
	if [ "$after_md5" = "$before_md5" ]; then
		echo "FAIL (unchanged)"
	else
		if [ "$before_md5" != "$bak_md5" ]; then
			echo "FAIL (bak different)"
		else
			echo PASS
		fi
	fi
fi


# Test the lack backup file if disabled.
echo -n "test lack of backup file ... "
rm -f $passfile.bak
use_config editor
echo "set backup no" >> test.config
$MDP -c test.config -e 2>/dev/null 1>/dev/null
before_md5=`get_md5 $passfile`
if [ -f "$passfile.bak" ]; then
	echo FAIL
else
	echo PASS
fi


# Cleanup.
rm -rf test.config test_gpg.batch test.output $GNUPGHOME
