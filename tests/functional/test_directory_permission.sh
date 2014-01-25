# Make sure mdp flames the user if their config/password directory has
# inappropriate permissions.
#
# We can't test the owner.
#

use_config simple

chmod 777 fake_gpg_home/.mdp
if run_mdp edit; then
	echo "expected EXIT_FAILURE"
	exit 1
fi
chmod 700 fake_gpg_home/.mdp

echo "mdp: bad permissions on fake_gpg_home/.mdp" > test.expected
assert_stderr
