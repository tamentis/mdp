#!/bin/sh

. ../_functions.sh

announce "config.c:config_resolve_character_set(\"anything\")"
./stub config_resolve_character_set anything > test.stdout || fail
echo "anything" > test.expected
assert_stdout && pass

announce "config.c:config_resolve_character_set(\"\$DIGITS\")"
./stub config_resolve_character_set '$DIGITS' > test.stdout || fail
echo "1234567890" > test.expected
assert_stdout && pass

announce "config.c:config_resolve_character_set(\"\$LOWERCASE\")"
./stub config_resolve_character_set '$LOWERCASE' > test.stdout || fail
echo "abcdefghijklmnopqrstuvwxyz" > test.expected
assert_stdout && pass

announce "config.c:config_resolve_character_set(\"\$UPPERCASE\")"
./stub config_resolve_character_set '$UPPERCASE' > test.stdout || fail
echo "ABCDEFGHIJKLMNOPQRSTUVWXYZ" > test.expected
assert_stdout && pass

announce "config.c:config_resolve_character_set(\"\$ALPHA\")"
./stub config_resolve_character_set '$ALPHA' > test.stdout || fail
echo "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" > test.expected
assert_stdout && pass

announce "config.c:config_resolve_character_set(\"\$ALPHANUMERIC\")"
./stub config_resolve_character_set '$ALPHANUMERIC' > test.stdout || fail
echo "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890" > test.expected
assert_stdout && pass

exit 0
