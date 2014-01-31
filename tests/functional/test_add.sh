# Test generating passwords with a default count (4)

# Create the initial file.
use_config simple
run_mdp edit

# Add the passwords lines.
use_config nop
run_mdp add -l 16 -n 3 my prefix

# Dump everything, replacing the passwords by dots
dump_password_file \
	| sed 's/	[a-zA-Z0-9]\{16\}/	................/' \
	> test.stdout

cat > test.expected << EOF
# my passwords
strawberry red
raspberry red
blackberry black
grapefruit yellow
my prefix	................
my prefix	................
my prefix	................
EOF

assert_stdout
