#!/bin/sh

. ../_functions.sh

announce "lock.c:lock_exists() - missing"
rm -f fake_lock
./stub fake_lock lock_exists > test.stdout
echo "0" > test.expected
assert_stdout && pass

announce "lock.c:lock_exists() - present"
touch fake_lock
./stub fake_lock lock_exists > test.stdout
echo "1" > test.expected
assert_stdout && pass

exit 0
