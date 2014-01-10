#!/bin/sh

. ../_functions.sh

announce "debug.c:debug()"
./stub something 2>&1 | awk '{print $4}' > output
echo "yup--something--" > expected
assert_output && pass

exit 0
