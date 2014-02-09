# Create the initial file.
use_config simple
run_mdp edit

# Add the passwords lines.
use_config nop
run_mdp add -l 18 -n 2

# Dump everything, replacing the passwords by dots
dump_password_file \
	| sed 's/[a-zA-Z0-9]\{18\}/................../' \
	> test.stdout

cat > test.expected << EOF
# my passwords
strawberry red
raspberry red
blackberry black
grapefruit yellow
..................
..................
EOF

assert_stdout
