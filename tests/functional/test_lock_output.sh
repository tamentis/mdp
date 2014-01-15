# Test the output of mdp if locked.

set +e

use_config slow

run_mdp -e > /dev/null &

sleep 0.1

OUTPUT=`run_mdp_capture_stderr -e`

if [ "$OUTPUT" = "mdp: locked (fake_gpg_home/.mdp/lock)" ]; then
	echo pass
fi

wait
