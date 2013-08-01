# Make sure the locks works properly (mdp returns non-0).

use_config sloweditor

run_mdp -e > /dev/null &

sleep 0.1

if ! run_mdp -e > /dev/null; then
	echo pass
fi

wait
