# Test the output of mdp if locked.

set +e

use_config slow

run_mdp edit > /dev/null &

sleep 0.1

run_mdp edit

echo "mdp: locked (fake_gpg_home/.mdp/lock)" > test.expected

assert_stderr

wait
