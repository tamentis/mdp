# Test the lack backup file if disabled.

rm -f $passfile.bak

# Create a default password file.
use_config simple
echo "set backup no" >> test.config
run_mdp edit

before_md5=`get_md5 $passfile`

if [ ! -f "$passfile.bak" ]; then
	echo pass
fi
