#!/bin/sh

. ../_functions.sh

announce debug
./stub something 2>&1 | awk '{print $5}' > output
echo "yup--something--" > expected
assert_output && pass

exit 0
