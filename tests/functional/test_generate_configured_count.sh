# Test generating 16 * 64 byte passwords

use_config editor

echo "set password_count 16" >> test.config

run_mdp -g 64 > test.output

if [ "`get_lines_and_bytes`" = "16 1040" ]; then
	echo pass
fi
