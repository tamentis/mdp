# Test generating passwords with a default count (4)

use_config simple
cat >> test.config << EOF
profile digits
  set character_set libertéсвобода自由
  set character_count 10
  set password_count 4
EOF

# Technically this is more a problem with the version of iconv than the
# operating system, but iconv --version is not exactly consistent so that will
# have to do for now.
if [ `uname` = "Linux" ]; then
	run_mdp generate -p digits \
		| iconv -f utf-8 -t ascii//TRANSLIT \
		| sed 's/[a-z?]/./g' \
		> test.stdout
else
	run_mdp generate -p digits \
		| iconv -f utf-8 -t ascii --unicode-subst=. \
		| sed 's/[a-z]/./g' \
		> test.stdout
fi

cat > test.expected << EOF
..........
..........
..........
..........
EOF

assert_stdout
