# Test generating 16 * 64 byte passwords

use_config editor

echo "profile default" >> test.config
echo "	set password_count 16" >> test.config

run_mdp -g -l 64 > test.stdout

if [ "`get_lines_and_bytes`" = "16 1040" ]; then
	echo pass
fi
