# Filter on two regex.

# Populate the password file.
use_config simple
run_mdp edit

run_mdp get -r -E berry 'red$' > test.stdout

cat > test.expected << EOF
strawberry red
raspberry red
EOF

assert_stdout
