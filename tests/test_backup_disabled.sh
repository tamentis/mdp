# Test the lack backup file if disabled.

rm -f $passfile.bak

use_config editor
echo "set backup no" >> test.config

run_mdp -e > /dev/null

before_md5=`get_md5 $passfile`

if [ ! -f "$passfile.bak" ]; then
	echo pass
fi
