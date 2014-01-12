#!/bin/sh

. ../_functions.sh

announce "utils.c:join_path()"

./stub join_path /etc passwd > test.stdout
echo "/etc/passwd" > test.expected
assert_stdout && pass

exit 0
