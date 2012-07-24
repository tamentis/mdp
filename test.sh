#!/bin/sh
#
# This is far from full coverage but it will serve as high level
# functional testing until better is crafted.
#

# This is called below as fake test editor.
if [ "$1" = "editor" ]; then
cat > $2 << EOF
strawberry red
raspberry red
blackberry black
grapefruit yellow
EOF
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
cat > test.config <<EOF
set gpg_path "/usr/local/bin/gpg"
set gpg_key_id "${key_id}"
set editor "`pwd`/test.sh editor"
set timeout 3
EOF

# First run to create a few password.
./mdp -c test.config -e 2>/dev/null

echo ok

echo -n "mdp -r red ... "
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
if ./mdp -c test.config -r red 2> /dev/null | diff - - << EOF
blackberry black
EOF
then
	echo SUCCESS
else
	echo FAIL
fi

echo -n "mdp berry black (timeout) ... "
start_ts=`date +%s`
./mdp -c test.config red 2>/dev/null 1>/dev/null
stop_ts=`date +%s`
if [[ "($stop_ts - $start_ts)" -eq 3 ]]; then
	echo SUCCESS
else
	echo FAIL
fi


# Cleanup.
rm -rf test.config test_gpg.batch $GNUPGHOME
