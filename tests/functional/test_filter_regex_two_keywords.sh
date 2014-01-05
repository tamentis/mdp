# Filter on two regex.

use_config editor

run_mdp -r -E berry 'red$' > test.stdout

if diff test.stdout - << EOF
strawberry red
raspberry red
EOF
then
	echo pass
fi
