#!/bin/sh

. ../_functions.sh

announce "str.c:join_list() - two arguments"
./stub join_list _ 2 first second > test.stdout
echo "first_second" > test.expected
assert_stdout && pass

announce "str.c:join_list() - five arguments"
./stub join_list _ 5 ab ra ca da bra > test.stdout
echo "ab_ra_ca_da_bra" > test.expected
assert_stdout && pass

exit 0
