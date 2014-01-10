#!/bin/sh

. ../_functions.sh

announce "utils.c:join_path()"

./stub join_path /etc passwd > output
echo "/etc/passwd" > expected
assert_output && pass

exit 0
