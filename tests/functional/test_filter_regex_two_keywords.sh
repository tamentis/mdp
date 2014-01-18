# Filter on two regex.

# Populate the password file.
use_config simple
run_mdp edit

run_mdp get -r -E berry 'red$' > test.stdout

if diff test.stdout - << EOF
strawberry red
raspberry red
EOF
then
	echo pass
fi
