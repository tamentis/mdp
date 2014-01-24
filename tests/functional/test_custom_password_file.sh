# Test the passwords_file parameter from the config file.

use_config simple
echo "set password_file fake_gpg_home/.mdp/alternative" >> test.config

rm -f fake_gpg_home/.mdp/passwords
rm -f fake_gpg_home/.mdp/alternative
run_mdp edit

assert_file_exists fake_gpg_home/.mdp/alternative
