# Filter on two keywords, test timeout setting (should pause for 3 seconds).

use_config simple

start_ts=`date +%s`
run_mdp red > /dev/null
stop_ts=`date +%s`

delta=$((stop_ts - start_ts))

if [ $delta -ge 2 ] || [ $delta -le 4 ]; then
	echo pass
fi
