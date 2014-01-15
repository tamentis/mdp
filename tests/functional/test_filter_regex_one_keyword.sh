# Filter on one regex.

# Populate the password file.
use_config simple
run_mdp -e

run_mdp -r -E ^.....berry > test.stdout

if diff test.stdout - << EOF
strawberry red
blackberry black
EOF
then
	echo pass
fi
