# Filter on red and expect all the lines containing red in the output.

use_config editor

run_mdp -r red > test.stdout

if diff test.stdout - << EOF
strawberry red
raspberry red
EOF
then
	echo pass
fi
