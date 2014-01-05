# Test generating 16 * 64 byte passwords

use_config editor

run_mdp -g -l 64 -n 16 > test.stdout

if [ "`get_lines_and_bytes`" = "16 1040" ]; then
	echo pass
fi
