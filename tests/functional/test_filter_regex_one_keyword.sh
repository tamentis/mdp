# Filter on one regex.

use_config editor

run_mdp -r -E ^.....berry > test.stdout

if diff test.stdout - << EOF
strawberry red
blackberry black
EOF
then
	echo pass
fi
