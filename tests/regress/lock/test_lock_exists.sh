#!/bin/sh

. ../_functions.sh

announce "lock_exists (missing)"
rm -f fake_lock
./stub fake_lock lock_exists > output
echo "0" > expected
assert_output && pass

announce "lock_exists (present)"
touch fake_lock
./stub fake_lock lock_exists > output
echo "1" > expected
assert_output && pass

exit 0
