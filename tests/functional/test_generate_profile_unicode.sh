# Test generating passwords with a default count (4)

use_config simple
cat >> test.config << EOF
profile digits
  set character_set libertéсвобода自由
  set character_count 10
  set password_count 4
EOF

run_mdp generate -p digits \
	| iconv -f utf-8 -t ascii --unicode-subst=. \
	| sed 's/[a-z]/./g' \
	> test.stdout

cat > test.expected << EOF
..........
..........
..........
..........
EOF

assert_stdout
