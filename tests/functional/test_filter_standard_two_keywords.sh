# Filter on two keywords, berry and black.

# Populate the password file.
use_config simple
run_mdp edit

run_mdp get -r berry black > test.stdout

cat > test.expected << EOF
blackberry black
EOF

assert_stdout
