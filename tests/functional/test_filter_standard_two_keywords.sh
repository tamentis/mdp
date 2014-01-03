# Filter on two keywords, berry and black.

use_config editor

run_mdp -r berry black > test.output

if diff test.output - << EOF
blackberry black
EOF
then
	echo pass
fi
