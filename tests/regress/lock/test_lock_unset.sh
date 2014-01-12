#!/bin/sh

. ../_functions.sh

announce "lock.c:lock_unset() - missing"
rm -f fake_lock
./stub fake_lock lock_unset
assert_retcode_success $?
assert_file_missing fake_lock && pass

announce "lock.c:lock_unset() - present"
touch fake_lock
./stub fake_lock lock_unset 2> test.stderr
assert_retcode_success $?
assert_file_missing fake_lock && pass

exit 0
