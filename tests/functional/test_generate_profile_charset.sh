# Test generating passwords with a default count (4)

use_config simple
cat >> test.config << EOF
profile digits
  set character_set 1234567890
  set character_count 10
  set password_count 4
EOF

run_mdp generate -p digits \
	| sed 's/[0-9]/./g' \
	> test.stdout

cat > test.expected << EOF
..........
..........
..........
..........
EOF

assert_stdout
