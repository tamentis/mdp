# Filter on two regex.

use_config editor

run_mdp -r -E berry 'red$' > test.output

if diff test.output - << EOF
strawberry red
raspberry red
EOF
then
	echo pass
fi
