# Test generating passwords with a default count (4)

use_config simple

run_mdp -g -l 16 > test.stdout

if [ "`get_lines_and_bytes test.stdout`" = "4 68" ]; then
	echo pass
fi
