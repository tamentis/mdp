# Filter on pass which is only found in a comment.

# Populate the password file.
use_config simple
run_mdp edit

run_mdp get -r pass > test.stdout

cat > test.expected << EOF
EOF

assert_stdout
