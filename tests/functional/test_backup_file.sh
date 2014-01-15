# Test the backup file.

# First run sets up a basic password file, then deletes the bak.
use_config alt
run_mdp -e > /dev/null
rm -f $passfile.bak


# Second runs should create a bak file.
use_config simple
run_mdp -e > /dev/null
if [ ! -f "$passfile.bak" ]; then
	echo "bak file not found"
	return
fi
before_md5=`get_md5 $passfile`


# Now we compare.
use_config alt

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
