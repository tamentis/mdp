#!/bin/sh

. ../_functions.sh

announce "str.c:join()"
./stub join _ first second > test.stdout
echo "first_second" > test.expected
assert_stdout && pass

exit 0
