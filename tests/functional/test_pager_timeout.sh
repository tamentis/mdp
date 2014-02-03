# Filter on two keywords, test timeout setting (should pause for 3 seconds).

# Create initial passwords.
use_config simple
run_mdp edit

# Time the pager when not given any input.
start_ts=`date +%s`
run_mdp get red > /dev/null
stop_ts=`date +%s`

delta=$((stop_ts - start_ts))

if [ $delta -ge 2 ] && [ $delta -le 4 ]; then
	echo pass
fi
