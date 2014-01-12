#!/bin/sh

. ../_functions.sh

announce "debug.c:debug()"
./stub something 2>&1 | awk '{print $4}' > test.stdout
echo "yup--something--" > test.expected
assert_stdout && pass

exit 0
