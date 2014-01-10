#!/bin/sh

. ../_functions.sh

announce "profile.c:profile_get_from_name()"
rm -f fake_lock

if ./stub get_from_name papple; then
	fail "missing profile returned something"
fi

if ! ./stub get_from_name banana; then
	fail "existing profile was not found"
fi

pass

exit 0
