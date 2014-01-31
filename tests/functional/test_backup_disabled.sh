# Test the lack backup file if disabled.

# Create a default password file.
use_config simple
run_mdp edit

rm -f "$passfile.bak"

# Edit the password file.
use_config alt
echo "set backup no" >> test.config
run_mdp edit

assert_file_absent "$passfile.bak"
