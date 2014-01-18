# Filter on red and expect all the lines containing red in the output.

# Populate the password file.
use_config simple
run_mdp edit

run_mdp get -r red > test.stdout

if diff test.stdout - << EOF
strawberry red
raspberry red
EOF
then
	echo pass
fi
