#!/bin/sh

. ../_functions.sh

announce "utils.c:join()"

./stub join _ first second > output
echo "first_second" > expected
assert_output && pass

exit 0
