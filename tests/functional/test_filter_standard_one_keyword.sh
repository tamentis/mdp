# Filter on red and expect all the lines containing red in the output.

use_config editor

run_mdp -r red > test.output

if diff test.output - << EOF
strawberry red
raspberry red
EOF
then
	echo pass
fi
