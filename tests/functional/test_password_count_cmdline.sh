# Test generating 16 * 64 byte passwords

use_config simple

run_mdp generate -l 64 -n 16 > test.stdout

if [ "`get_lines_and_bytes`" = "16 1040" ]; then
	echo pass
fi
