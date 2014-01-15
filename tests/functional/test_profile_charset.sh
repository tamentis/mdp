# Test generating a password made of custom characters.

use_config simple

echo "profile wat" >> test.config
echo "	set character_set 01234" >> test.config
echo "	set character_count 10" >> test.config

run_mdp -g -p wat -l 10 > test.tmp

grep '^[01234]\{10\}$' test.tmp > test.stdout
rm -f test.tmp
if [ "`get_lines_and_bytes`" = "4 44" ]; then
	echo pass
fi
