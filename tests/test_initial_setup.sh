# First run to create a few password. We test the return code of this since
# failing to create the initial password would cause this to die.

use_config editor

if run_mdp -e; then
	echo pass
fi
