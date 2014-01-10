# Test -h works.

use_config editor

if run_mdp -h > test.stdout; then
	echo "fail (wrong return code on -h)"
	exit 1
fi

if ! grep usage: test.stdout > /dev/null; then
	echo "fail (help does not contain usage string)"
	exit 1
fi

echo pass
exit 0
