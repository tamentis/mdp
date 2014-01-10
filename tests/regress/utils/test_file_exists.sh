#!/bin/sh

. ../_functions.sh

FILENAME=any_random_file

announce "utils.c:file_exists() - missing"
touch $FILENAME
if ! ./stub file_exists $FILENAME; then
	fail "file exists and was not found"
fi
pass

announce "utils.c:file_exists() - exists"
rm -f $FILENAME
if ./stub file_exists $FILENAME; then
	fail "file does not exists and was detected as existing"
fi
pass

exit 0
