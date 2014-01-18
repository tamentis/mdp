# Filter on two keywords, berry and black.

# Populate the password file.
use_config simple
run_mdp edit

run_mdp get -r berry black > test.stdout

if diff test.stdout - << EOF
blackberry black
EOF
then
	echo pass
fi
