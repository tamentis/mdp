# Filter on one regex.

# Populate the password file.
use_config simple
run_mdp edit

run_mdp get -r -E ^.....berry > test.stdout

cat > test.expected << EOF
strawberry red
blackberry black
EOF

assert_stdout
