# Test generating latin1 passwords.

# Assume old school ISO locale for this.
export LANG=fr_FR.ISO8859-1
export LC_ALL=$LANG

use_config simple
cat >> test.config << EOF
profile digits
  set character_set libertéàøîïb
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
	alias latin1_to_ascii="iconv -f latin1 -t ascii --unicode-subst='?'"
	;;
FreeBSD|NetBSD)
	# Custom iconv implementation coming from NetBSD.
	alias latin1_to_ascii="iconv -s -f latin1 -t ASCII"
	;;
Linux|*)
	# Newer GNU iconv.
	alias latin1_to_ascii="iconv -f latin1 -t ASCII//TRANSLIT"
	;;
esac

run_mdp generate -p digits \
	| latin1_to_ascii \
	| sed 's/[a-z?]/./g' \
	> test.stdout

cat > test.expected << EOF
..........
..........
..........
..........
EOF

assert_stdout
