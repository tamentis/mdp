# Filter on two keywords, berry and black.

use_config editor

run_mdp -r berry black > test.stdout

if diff test.stdout - << EOF
blackberry black
EOF
then
	echo pass
fi
