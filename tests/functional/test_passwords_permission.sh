# Make sure mdp flames the user if their passwords file has inappropriate
# permissions.
#
# NOTE: We can't test ownership.
#

use_config simple

# Create a file.
run_mdp edit

chmod 666 fake_gpg_home/.mdp/passwords
if run_mdp edit; then
	echo "expected EXIT_FAILURE"
	exit 1
fi
chmod 600 fake_gpg_home/.mdp/passwords

echo "mdp: bad permissions on fake_gpg_home/.mdp/passwords" > test.expected
assert_stderr
