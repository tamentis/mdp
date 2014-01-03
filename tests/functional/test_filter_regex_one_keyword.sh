# Filter on one regex.

use_config editor

run_mdp -r -E ^.....berry > test.output

if diff test.output - << EOF
strawberry red
blackberry black
EOF
then
	echo pass
fi
