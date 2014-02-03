# Test generating passwords with a default count (4)

# Assume UTF-8 locale for this, if you use international characters without
# UTF-8, feel free to fix or file a bug report.
export LANG=en_US.UTF-8
export LC_ALL=$LANG

use_config simple
cat >> test.config << EOF
profile digits
  set character_set libertéсвобода自由
  set character_count 10
  set password_count 4
EOF

# Technically this is more a problem with the version of iconv than the
# operating system, but iconv --version is not exactly consistent so that will
# have to do for now. TODO: write something to detect the implementation of
# iconv.
case `uname` in
OpenBSD|Darwin)
	# Older GNU iconv.
	alias utf_to_ascii="iconv -f utf-8 -t ascii --unicode-subst='?'"
	;;
FreeBSD|NetBSD)
	# Custom iconv implementation coming from NetBSD.
	alias utf_to_ascii="iconv -s -f UTF-8 -t ASCII"
	;;
Linux|*)
	# Newer GNU iconv.
	alias utf_to_ascii="iconv -f UTF-8 -t ASCII//TRANSLIT"
	;;
esac

run_mdp generate -p digits \
	| utf_to_ascii \
	| sed 's/[a-z?]/./g' \
	> test.stdout

cat > test.expected << EOF
..........
..........
..........
..........
EOF

assert_stdout
