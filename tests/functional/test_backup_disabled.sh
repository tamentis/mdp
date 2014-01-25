# Test the lack backup file if disabled.

rm -f "$passfile.bak"

# Create a default password file.
use_config simple
run_mdp edit

# Edit the password file.
use_config alt
echo "set backup no" >> test.config
run_mdp edit

assert_file_absent "$passfile.bak"
