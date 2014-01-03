#!/bin/sh

. ../_functions.sh

announce "lock_set (missing)"
rm -f fake_lock
./stub fake_lock lock_set
assert_retcode_success $?
assert_file_exists fake_lock && pass

announce "lock_set (present)"
touch fake_lock
./stub fake_lock lock_set 2>output
assert_retcode_failure $?
echo "stub: locked (fake_lock)" > expected
assert_output && pass

exit 0
