# Make sure the locks works properly (mdp returns non-0).

use_config slow

run_mdp edit > /dev/null &

sleep 0.1

if ! run_mdp edit > /dev/null; then
	echo pass
fi

wait
