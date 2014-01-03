# Test generating passwords with a default count (4)

use_config editor

run_mdp -g 16 > test.output

if [ "`get_lines_and_bytes test.output`" = "4 68" ]; then
	echo pass
fi
