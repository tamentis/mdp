# Filter on pass which is only found in a comment.

# Populate the password file.
use_config simple
run_mdp -e

run_mdp -r pass > test.stdout

if diff test.stdout - << EOF
EOF
then
	echo pass
fi
