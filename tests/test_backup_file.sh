# Test the backup file.

rm -f $passfile.bak

use_config editor

run_mdp -e > /dev/null

before_md5=`get_md5 $passfile`

if [ ! -f "$passfile.bak" ]; then
	echo "bak file not found"
	return
fi

use_config alt_editor

run_mdp -e > /dev/null

after_md5=`get_md5 $passfile`
bak_md5=`get_md5 $passfile.bak`

if [ "$after_md5" = "$before_md5" ]; then
	echo "password file unchanged"
	return
fi

if [ "$before_md5" = "$bak_md5" ]; then
	echo pass
fi
