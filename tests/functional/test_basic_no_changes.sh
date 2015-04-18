# Create a few passwords
use_config simple
run_mdp edit

# Run mdp without edits
use_config nop
run_mdp edit

# Only keep the last line of stderr, that's all we care about for this test.
tail -n 1 test.stderr > test.tmp
mv test.tmp test.stderr

cat > test.expected <<EOF
No changes, exiting...
EOF

assert_stderr
